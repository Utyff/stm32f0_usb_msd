#include <stm32f0xx.h>
#include "usb_lib.h"
#include "at45db161d.h"

#define INQUIRY							0x12
#define REQUEST_SENSE					0x03
#define READ_CAPACITY_10				0x25
#define READ_10							0x28
#define TEST_UNIT_READY					0x00
#define MODE_SENSE_6					0x1A
#define VERIFY_10						0x2F
#define PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define WRITE_10						0x2A

#define SCSI_REQUEST					0
#define SCSI_ANSWER						1
#define SCSI_CSW						2

typedef struct {
	unsigned read_ok : 1;
	unsigned write_ok : 1;
} _scsi_status_t;

extern uint8_t scsi_status;
extern _scsi_status_t scsi_stat;

typedef struct {
	uint8_t opcode;
	uint8_t CMDDT_EVPD;
	uint8_t PageCode;
	uint16_t length;
	uint8_t control;
} cbw_inquiry_t;

typedef struct {
	uint16_t dCBWSignatureL;
	uint16_t dCBWSignatureH;
	uint16_t dCBWTagL;
	uint16_t dCBWTagH;
	uint16_t dCBWDataTransferLengthL;
	uint16_t dCBWDataTransferLengthH;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
} scsi_cbw_t;

typedef struct {
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
} scsi_csw_t;

void SCSI_Execute(uint8_t ep_number);

