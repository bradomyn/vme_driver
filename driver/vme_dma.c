/*
 * vme_dma.c - PCI-VME bridge DMA management
 *
 * Copyright (c) 2009 Sebastien Dugue
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

/*
 *  This file provides the PCI-VME bridge DMA management code.
 */

#include <linux/pagemap.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif

#include <asm/atomic.h>
#include <linux/delay.h> 

#include "vmebus.h"
#include "vme_bridge.h"
#include "vme_dma.h"

struct dma_channel channels[TSI148_NUM_DMA_CHANNELS];

/*
 * @dma_semaphore manages the common queue to access all the DMA channels.
 * Once a process gets through the semaphore, it must acquire
 * dma_lock mutex to atomically look for an available channel.
 * The @disable flag can be set to disable any further DMA transfers.
 */
static struct semaphore	dma_semaphore;
static struct mutex	dma_lock;
static atomic_t		dma_disable;

/*
 * Used for synchronizing between DMA transfer using a channel and
 * module exit
 */
wait_queue_head_t channel_wait[TSI148_NUM_DMA_CHANNELS];

void handle_dma_interrupt(int channel_mask)
{
	if (channel_mask & 1)
		wake_up(&channels[0].wait);

	if (channel_mask & 2)
		wake_up(&channels[1].wait);

	account_dma_interrupt(channel_mask);
}


static int sgl_fill_user_pages(struct page **pages, unsigned long uaddr,
			const unsigned int nr_pages, int rw)
{
	int ret;

	/* Get user pages for the DMA transfer */
	down_read(&current->mm->mmap_sem);
	ret = get_user_pages(current, current->mm, uaddr, nr_pages, rw, 0,
			    pages, NULL);
	up_read(&current->mm->mmap_sem);

	return ret;
}

static int sgl_fill_kernel_pages(struct page **pages, unsigned long kaddr,
			const unsigned int nr_pages, int rw)
{
	int i;

	/* Note: this supports lowmem pages only */
	if (!virt_addr_valid(kaddr))
		return -EINVAL;

	for (i = 0; i < nr_pages; i++)
		pages[i] = virt_to_page(kaddr + PAGE_SIZE * i);

	return nr_pages;
}

/**
 * sgl_map_user_pages() - Pin user pages and put them into a scatter gather list
 * @sgl: Scatter gather list to fill
 * @nr_pages: Number of pages
 * @uaddr: User buffer address
 * @count: Length of user buffer
 * @rw: Direction (0=read from userspace / 1 = write to userspace)
 * @to_user: 1 - transfer is to/from a user-space buffer. 0 - kernel buffer.
 *
 *  This function pins the pages of the userspace buffer and fill in the
 * scatter gather list.
 */
static int sgl_map_user_pages(struct scatterlist *sgl,
			      const unsigned int nr_pages, unsigned long uaddr,
			      size_t length, int rw, int to_user)
{
	int rc;
	int i;
	struct page **pages;

	if ((pages = kmalloc(nr_pages * sizeof(struct page *),
			     GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if (to_user) {
		rc = sgl_fill_user_pages(pages, uaddr, nr_pages, rw);
		if (rc >= 0 && rc < nr_pages) {
			/* Some pages were pinned, release these */
			for (i = 0; i < rc; i++)
				page_cache_release(pages[i]);
			rc = -ENOMEM;
			goto out_free;
		}
	} else {
		rc = sgl_fill_kernel_pages(pages, uaddr, nr_pages, rw);
	}

	if (rc < 0)
		/* We completely failed to get the pages */
		goto out_free;

	/* Populate the scatter/gather list */
	sg_init_table(sgl, nr_pages);

	/* Take a shortcut here when we only have a single page transfer */
	if (nr_pages > 1) {
		unsigned int off = offset_in_page(uaddr);
		unsigned int len = PAGE_SIZE - off;

		sg_set_page (&sgl[0], pages[0], len, off);
		length -= len;

		for (i = 1; i < nr_pages; i++) {
			sg_set_page (&sgl[i], pages[i],
				     (length < PAGE_SIZE) ? length : PAGE_SIZE,
				     0);
			length -= PAGE_SIZE;
		}
	} else
		sg_set_page (&sgl[0], pages[0], length, offset_in_page(uaddr));

out_free:
	/* We do not need the pages array anymore */
	kfree(pages);

	return nr_pages;
}

/**
 * sgl_unmap_user_pages() - Release the scatter gather list pages
 * @sgl: The scatter gather list
 * @nr_pages: Number of pages in the list
 * @dirty: Flag indicating whether the pages should be marked dirty
 * @to_user: 1 when transfer is to/from user-space (0 for to/from kernel)
 *
 */
static void sgl_unmap_user_pages(struct scatterlist *sgl,
				 const unsigned int nr_pages, int dirty,
				 int to_user)
{
	int i;

	if (!to_user)
		return;

	for (i = 0; i < nr_pages; i++) {
		struct page *page = sg_page(&sgl[i]);

		if (dirty && !PageReserved(page))
			SetPageDirty(page);

		page_cache_release (page);
	}
}

/**
 * vme_dma_setup() - Setup a DMA transfer
 * @desc: DMA channel to setup
 * @to_user:	1 if the transfer is to/from a user-space buffer.
 *		0 if it is to/from a kernel buffer.
 *
 *  Setup a DMA transfer.
 *
 *  Returns 0 on success, or a standard kernel error code on failure.
 */
static int vme_dma_setup(struct dma_channel *channel, int to_user)
{
	int rc = 0;
	struct vme_dma *desc = &channel->desc;
	unsigned int length = desc->length;
	unsigned int uaddr;
	int nr_pages;

	/* Create the scatter gather list */
	uaddr = (desc->dir == VME_DMA_TO_DEVICE) ?
		desc->src.addrl : desc->dst.addrl;

	/* Check for overflow */
	if ((uaddr + length) < uaddr)
		return -EINVAL;

	nr_pages = ((uaddr & ~PAGE_MASK) + length + ~PAGE_MASK) >> PAGE_SHIFT;

	if ((channel->sgl = kmalloc(nr_pages * sizeof(struct scatterlist),
				    GFP_KERNEL)) == NULL)
		return -ENOMEM;

	/* Map the user pages into the scatter gather list */
	channel->sg_pages = sgl_map_user_pages(channel->sgl, nr_pages, uaddr,
					       length,
					       (desc->dir==VME_DMA_FROM_DEVICE),
					       to_user);

	if (channel->sg_pages <= 0) {
		rc = channel->sg_pages;
		goto out_free_sgl;
	}

	/* Map the sg list entries onto the PCI bus */
	channel->sg_mapped = pci_map_sg(vme_bridge->pdev, channel->sgl,
					channel->sg_pages, desc->dir);

	rc = tsi148_dma_setup(channel);


	if (rc)
		goto out_unmap_sgl;

	return 0;

out_unmap_sgl:
	pci_unmap_sg(vme_bridge->pdev, channel->sgl, channel->sg_mapped,
		     desc->dir);

	sgl_unmap_user_pages(channel->sgl, channel->sg_pages, 0, to_user);

out_free_sgl:
	kfree(channel->sgl);

	return rc;
}

/**
 * vme_dma_start() - Start a DMA transfer
 * @channel: DMA channel to start
 *
 */
static void vme_dma_start(struct dma_channel *channel)
{
	/* Not much to do here */
	tsi148_dma_start(channel);
}

/* This function has to be called with dma_semaphore and dma_lock held. */
static struct dma_channel *__lock_avail_channel(void)
{
	struct dma_channel *channel;
	int i;

	for (i = 0; i < TSI148_NUM_DMA_CHANNELS; i++) {
		channel = &channels[i];

		if (!channel->busy) {
			channel->busy = 1;
			return channel;
		}
	}
	WARN_ON_ONCE(i == TSI148_NUM_DMA_CHANNELS);
	return ERR_PTR(-EDEADLK);
}

/*
 * Wait in the queue of the semaphore for an available channel. Then find
 * this newly available channel, and acquire it by flagging it as busy.
 */
static struct dma_channel *vme_dma_channel_acquire(void)
{
	struct dma_channel *channel;
	int rc;

	/* do not process any requests if dma_disable is set */
	if (atomic_read(&dma_disable))
		return ERR_PTR(-EBUSY);

	/* wait for a channel to be available */
	rc = down_interruptible(&dma_semaphore);
	if (rc)
		return ERR_PTR(rc);

	/*
	 * dma_disable might have been flagged while this task was
	 * sleeping on dma_semaphore.
	 */
	if (atomic_read(&dma_disable)) {
		up(&dma_semaphore);
		return ERR_PTR(-EBUSY);
	}

	/* find the available channel */
	mutex_lock(&dma_lock);
	channel = __lock_avail_channel();
	mutex_unlock(&dma_lock);

	return channel;
}

static void vme_dma_channel_release(struct dma_channel *channel)
{
	/* release the channel busy flag */
	mutex_lock(&dma_lock);
	channel->busy = 0;
	mutex_unlock(&dma_lock);

	/* up the DMA semaphore to mark there's a channel available */
	up(&dma_semaphore);
}

/*
 * @to_user:	1 - the transfer is to/from a user-space buffer
 *		0 - the transfer is to/from a kernel buffer
 */
static int __vme_do_dma(struct vme_dma *desc, int to_user)
{
	int rc = 0;
	struct dma_channel *channel;

	/* First check the transfer length */
	if (!desc->length) {
		printk(KERN_ERR PFX "%s: Wrong length %d\n",
		       __func__, desc->length);
		return -EINVAL;
	}

	/* Check the transfer direction validity */
	if ((desc->dir != VME_DMA_FROM_DEVICE) &&
	    (desc->dir != VME_DMA_TO_DEVICE)) {
		printk(KERN_ERR PFX "%s: Wrong direction %d\n",
		       __func__, desc->dir);
		return -EINVAL;
	}

	/* Check we're within a 32-bit address space */
	if (desc->src.addru || desc->dst.addru) {
		printk(KERN_ERR PFX "%s: Addresses are not 32-bit\n", __func__);
		return -EINVAL;
	}

	/* Acquire an available channel */
	channel = vme_dma_channel_acquire();
	if (IS_ERR(channel))
		return PTR_ERR(channel);

	memcpy(&channel->desc, desc, sizeof(struct vme_dma));

	/* Setup the DMA transfer */
	rc = vme_dma_setup(channel, to_user);

	if (rc)
		goto out_release_channel;

	/* Start the DMA transfer */
	vme_dma_start(channel);

	/* Wait for DMA completion */
	rc = wait_event_interruptible(channel->wait,
				 !tsi148_dma_busy(channel));

	/* React to user-space signals by aborting the ongoing DMA transfer */
	if (rc) {
		tsi148_dma_abort(channel);
		/* leave some time for the bridge to clear the DMA channel */
		udelay(10);
	}

	desc->status = tsi148_dma_get_status(channel);

	/* Now do some cleanup and we're done */
	tsi148_dma_release(channel);

	pci_unmap_sg(vme_bridge->pdev, channel->sgl, channel->sg_mapped,
		     desc->dir);

	sgl_unmap_user_pages(channel->sgl, channel->sg_pages, 0, to_user);

	kfree(channel->sgl);

out_release_channel:
	vme_dma_channel_release(channel);

	/* Signal we're done in case we're in module exit */
	wake_up(&channel_wait[channel->num]);

	return rc;
}

/**
 * vme_do_dma() - Do a DMA transfer
 * @desc: DMA transfer descriptor
 *
 *  This function first checks the validity of the user supplied DMA transfer
 * parameters. It then tries to find an available DMA channel to do the
 * transfer, setups that channel and starts the DMA.
 *
 *  Returns 0 on success, or a standard kernel error code on failure.
 */
int vme_do_dma(struct vme_dma *desc)
{
	return __vme_do_dma(desc, 1);
}
EXPORT_SYMBOL_GPL(vme_do_dma);

/**
 * vme_do_dma_kernel() - Do a DMA transfer to/from a kernel buffer
 * @desc: DMA transfer descriptor
 *
 * Returns 0 on success, or a standard kernel error code on failure.
 */
int vme_do_dma_kernel(struct vme_dma *desc)
{
	return __vme_do_dma(desc, 0);
}
EXPORT_SYMBOL_GPL(vme_do_dma_kernel);

/**
 * vme_dma_ioctl() - ioctl file method for the VME DMA device
 * @file: Device file descriptor
 * @cmd: ioctl number
 * @arg: ioctl argument
 *
 *  Currently the VME DMA device supports the following ioctl:
 *
 *    VME_IOCTL_START_DMA
 */
long vme_dma_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	struct vme_dma desc;
	void __user *argp = (void __user *)arg;

	switch (cmd) {
	case VME_IOCTL_START_DMA:
		/* Get the DMA transfer descriptor */
		if (copy_from_user(&desc, (void *)argp, sizeof(struct vme_dma)))
			return -EFAULT;

		/* Do the DMA */
		rc = vme_do_dma(&desc);

		if (rc)
			return rc;

		/*
		 * Copy back the DMA transfer descriptor containing the DMA
		 * updated status.
		 */
		if (copy_to_user((void *)argp, &desc, sizeof(struct vme_dma)))
			return -EFAULT;

		break;

	default:
		rc = -ENOIOCTLCMD;
	}


	return rc;
}

/**
 * vme_dma_exit() - Release DMA management resources
 *
 *
 */
void __devexit vme_dma_exit(void)
{
	int i;

	/* do not perform any further DMA operations */
	atomic_set(&dma_disable, 1);

	/* abort all the in flight DMA operations */
	for (i = 0; i < TSI148_NUM_DMA_CHANNELS; i++) {
		tsi148_dma_abort(&channels[i]);
	}

	/* wait until all the channels are idle */
	for (i = 0; i < TSI148_NUM_DMA_CHANNELS; i++) {
		down(&dma_semaphore);
		up(&dma_semaphore);
	}

	tsi148_dma_exit();
}

/**
 * vme_dma_init() - Initialize DMA management
 *
 */
int __devinit vme_dma_init(void)
{
	int i;

	for (i = 0; i < TSI148_NUM_DMA_CHANNELS; i++) {
		channels[i].num = i;
		init_waitqueue_head(&channels[i].wait);
		init_waitqueue_head(&channel_wait[i]);
		INIT_LIST_HEAD(&channels[i].hw_desc_list);
	}

	sema_init(&dma_semaphore, TSI148_NUM_DMA_CHANNELS);
	mutex_init(&dma_lock);
	atomic_set(&dma_disable, 0);
	return tsi148_dma_init();
}
