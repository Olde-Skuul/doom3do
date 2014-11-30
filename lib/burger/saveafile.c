#include "Burger.h"
#include <filesystem.h>
#include <filefunctions.h>
#include <string.h>
#include <operror.h>

/**********************************

	Save a file to NVRAM or disk

**********************************/

Word SaveAFile(Byte *name,void *data,LongWord dataSize)
{
	Item fileItem;		/* Item for opened file */
	Item ioReqItem;		/* Item for i/o request */
	IOInfo ioInfo;		/* Struct for I/O request data passing */
	Err result;			/* Returned result */
	LongWord numBlocks;	/* Number of blocks returned */
	LongWord blockSize;	/* Size of a data block for device */
	LongWord roundedSize;	/* Rounded file size */
	FileStatus status;	/* Status I/O request record */

	DeleteFile((char *)name);	/* Get rid of the file if it was already there */

	result = CreateFile((char *)name);		/* Create the file again... */
	if (result >= 0) {
		fileItem = OpenDiskFile((char *)name);	/* Open the file for access */
		if (fileItem >= 0) {

			/* Create an IOReq to communicate with the file */

			ioReqItem = CreateIOReq(NULL,0,fileItem,0);
			if (ioReqItem >= 0) {
				/* Get the block size of the file */
				memset(&ioInfo, 0, sizeof(IOInfo));
				ioInfo.ioi_Command = CMD_STATUS;
				ioInfo.ioi_Recv.iob_Buffer = &status;
				ioInfo.ioi_Recv.iob_Len = sizeof(FileStatus);
				result = DoIO(ioReqItem,&ioInfo);
				if (result >= 0) {
					blockSize = status.fs.ds_DeviceBlockSize;
					/* Round to block size */

					numBlocks = (dataSize + blockSize - 1) / blockSize;

					/* allocate the blocks we need for this file */
					ioInfo.ioi_Command = FILECMD_ALLOCBLOCKS;
					ioInfo.ioi_Recv.iob_Buffer = NULL;
					ioInfo.ioi_Recv.iob_Len = 0;
					ioInfo.ioi_Offset = numBlocks;
					result = DoIO(ioReqItem, &ioInfo);
					if (result >= 0) {
						/* Tell the system how many bytes for this file */
						memset(&ioInfo,0,sizeof(IOInfo));
						ioInfo.ioi_Command = FILECMD_SETTYPE;
						ioInfo.ioi_Offset = (LongWord)0x33444F46;
						DoIO(ioReqItem,&ioInfo);

						memset(&ioInfo,0,sizeof(IOInfo));
						ioInfo.ioi_Command = FILECMD_SETEOF;
						ioInfo.ioi_Offset = dataSize;
						result = DoIO(ioReqItem, &ioInfo);
						if (result >= 0) {
							roundedSize = 0;
							if (dataSize >= blockSize) {
								/* If we have more than one block's worth of */
								/* data, write as much of it as possible. */

								roundedSize = (dataSize / blockSize) * blockSize;
								ioInfo.ioi_Command = CMD_WRITE;
								ioInfo.ioi_Send.iob_Buffer = (void *)data;
								ioInfo.ioi_Send.iob_Len = roundedSize;
								ioInfo.ioi_Offset = 0;
								result = DoIO(ioReqItem, &ioInfo);

								data = (void *)((LongWord)data + roundedSize);
								dataSize -= roundedSize;	/* Data remaining */
							}

			/* If the amount of data left isn't as large as a whole */
			/* block, we must allocate a memory buffer of the size */
			/* of the block, copy the rest of the data into it, */
			/* and write the buffer to disk. */

							if ((result >= 0) && dataSize) {
								void *temp;
								temp = AllocAPointer(blockSize);
								if (temp) {
									memcpy(temp,data,dataSize);
									ioInfo.ioi_Command = CMD_WRITE;
									ioInfo.ioi_Send.iob_Buffer = temp;
									ioInfo.ioi_Send.iob_Len = blockSize;
									ioInfo.ioi_Offset = roundedSize;
									result = DoIO(ioReqItem,&ioInfo);

									DeallocAPointer(temp);
								} else {
									result = NOMEM;
								}
							}
						}
					}
				}
				DeleteIOReq(ioReqItem);
			} else {
				result = ioReqItem;
			}
			CloseDiskFile(fileItem);
		} else {
			result = fileItem;
		}

		/* don't leave a potentially corrupt file around... */
		if (result < 0) {
			DeleteFile((char *)name);
		}
	}
	if (result>=0) {
		result = 0;
	}
	return (result);
}
