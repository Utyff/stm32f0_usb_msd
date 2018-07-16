#include "scsi.h"

uint8_t buf[512];

uint8_t inquiry[36] = {
		0x00, 			//Block device
		0x80,			//Removable media
		0x04,			//SPC-3
		0x02,			//Response data format = 0x02
		0x1F,			//Additional_length = length - 5
		0x00,
		0x00,
		0x00,
		'S', 'O', 'B', ' ', 'i', 'n', 'c', '.',
		'M', 'a', 's', 's', ' ', 'S', 't', 'o', 'r', 'a', 'g', 'e', ' ', ' ', ' ', ' ',
		'0', '0', '0', '1'
};

uint8_t sense_data[18] = {
		0x70,		//VALID = 1, RESRONSE_CODE = 0x70
		0x00,
		0x05,		//S_ILLEGAL_REQUEST
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t capacity[8] = {
		0x00, 0x00, 0x0F, 0xFF,		//Addr last blocks
		0x00, 0x00, 0x02, 0x00		//Size blocks = 512 bytes
};

uint8_t mode_sense_6[4] = {
		0x03, 0x00, 0x00, 0x00,
};

scsi_csw_t CSW = {
	0x53425355,
	0,
	0,
	0
};

uint8_t scsi_status;
_scsi_status_t scsi_stat;

void SCSI_Execute(uint8_t ep_number){
	uint32_t i, n;
	uint32_t status;
	uint8_t j;
	scsi_cbw_t *cbw = (scsi_cbw_t *)endpoints[ep_number].rx_buf;
	if (endpoints[ep_number].rx_flag){
		CSW.dCSWTag = (cbw -> dCBWTagH << 16) | cbw -> dCBWTagL;
		switch (cbw -> CBWCB[0]){
		case INQUIRY:
			if (cbw -> CBWCB[1] == 0){
				EP_Write(ep_number, inquiry, cbw -> CBWCB[4]);
				CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL) - cbw -> CBWCB[4];
				CSW.bCSWStatus = 0x00;
				EP_Write(ep_number, (uint8_t *)&CSW, 13);
			} else {
				CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL);
				CSW.bCSWStatus = 0x01;
				EP_Write(ep_number, (uint8_t *)&CSW, 13);
				CSW.bCSWStatus = 0x00;
				EP_Write(ep_number, (uint8_t *)&CSW, 13);
			}
			break;
		case REQUEST_SENSE:
			EP_Write(ep_number, sense_data, 18);
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL) - cbw -> CBWCB[4];
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		case READ_CAPACITY_10:
			EP_Write(ep_number, capacity, 8);
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL) - cbw -> CBWCB[4];
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		case MODE_SENSE_6:
			EP_Write(ep_number, mode_sense_6, 4);
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL) - cbw -> CBWCB[4];
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		case READ_10:
			i = ((cbw -> CBWCB[2] << 24) | (cbw -> CBWCB[3] << 16) | (cbw -> CBWCB[4] << 8) | (cbw -> CBWCB[5]));
			n = i + ((cbw -> CBWCB[7] << 8) | cbw -> CBWCB[8]);
			for ( ; i < n; i++){
				AT45DB161_Read_Data(i, 0, 512, buf);
				for (j = 0; j < 8; j++){
					EP_Write(ep_number, (uint8_t *)&buf[64*j], 64);
				}
			}
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL) - cbw -> CBWCB[4];
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		case WRITE_10:
			i = ((cbw -> CBWCB[2] << 24) | (cbw -> CBWCB[3] << 16) | (cbw -> CBWCB[4] << 8) | (cbw -> CBWCB[5]));
			n = i + ((cbw -> CBWCB[7] << 8) | cbw -> CBWCB[8]);
			for ( ; i < n; i++){
				for (j = 0; j < 8; j++){
					EP_Read(ep_number, (uint8_t *)&buf[64*j]);
				}
				AT45DB161_PageProgram(i, buf, 512);
			}
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL) - cbw -> CBWCB[4];
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		case TEST_UNIT_READY:
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL);
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		case PREVENT_ALLOW_MEDIUM_REMOVAL:
			CSW.dCSWDataResidue = 0;
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		default:
			CSW.dCSWDataResidue = ((cbw -> dCBWDataTransferLengthH << 16) | cbw -> dCBWDataTransferLengthL);
			CSW.bCSWStatus = 0x01;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			CSW.bCSWStatus = 0x00;
			EP_Write(ep_number, (uint8_t *)&CSW, 13);
			break;
		}
		status = USB -> EPnR[ep_number];
		status = SET_VALID_RX(status);
		status = SET_NAK_TX(status);
		status = KEEP_DTOG_TX(status);
		status = KEEP_DTOG_RX(status);
		USB -> EPnR[ep_number] = status;
		endpoints[ep_number].rx_flag = 0;
		endpoints[ep_number].tx_flag = 0;
	}
}
