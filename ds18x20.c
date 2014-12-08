#include "ds18x20.h"

#include <avr/pgmspace.h>

#include "Charset.h"

//==============================================================
// CONSTANTS AND GLOBAL VARIABLES
//==============================================================

#define DS18_CMD_CONVERT_T         0x44
#define DS18_CMD_READ              0xBE
#define DS18_CMD_WRITE             0x4E
#define DS18_CMD_EE_WRITE          0x48
#define DS18_CMD_EE_RECALL         0xB8
#define DS18_CMD_READ_POWER_SUPPLY 0xB4

//==============================================================
// PUBLIC PROCEDURES
//==============================================================

//*********************************
//wykryj wszystkie termometry i wyœwietl ich IDki na LCD
void DS18Detect(void) {
	//CSPrintV_p(PSTR("Rozpoczynam skanowanie"));
	uint8_t vi_Diff, vi_Idx, vi_Idx2;
	if (!DS18MAXCOUNT)
		CSPrintV_p(PSTR("Zwiêksz DS18MAXCOUNT w ds18x20.h"));
	gi_DS18Count = 0;
	for (vi_Diff = OW_SEARCH_FIRST; vi_Diff != OW_LAST_DEVICE && gi_DS18Count < DS18MAXCOUNT;) {
		vi_Diff = ow_rom_search(vi_Diff, &gt_DS18IDs[gi_DS18Count][0]);
		if (vi_Diff == OW_PRESENCE_ERR || vi_Diff == OW_DATA_ERR || gi_DS18Count == DS18MAXCOUNT) {
			break;
		}
		gi_DS18Count++;
	}
	if (gi_DS18Count == DS18MAXCOUNT) {
		CSPrintV_p(PSTR("Zwiêksz DS18MAXCOUNT w ds18x20.h"));
		CSNewLine();
	}
	for (vi_Idx = 0; vi_Idx < gi_DS18Count; vi_Idx++) {
		for (vi_Idx2 = 0; vi_Idx2 < 8; vi_Idx2++) {
			CSPrintV(CSHex2V(&gt_DS18IDs[vi_Idx][vi_Idx2],1));
		}
		CSNewLine();
	}
} //DS18Detect

//*********************************
//rozpocznij pomiar temperatury
void DS18StartMeas(void) {
	ow_command(DS18_CMD_CONVERT_T, 0, 0); //with SKIP_ROM
} //DS18StartMeas

//*********************************
//odczytaj temperaturê pojedynczego termometru (0 - jeœli jeden termometr)
int16_t DS18ReadMeas(uint8_t pt_TempId[], uint8_t pb_Progmem) {
	union {
		int16_t W;
		struct {
			uint8_t L, H;
		} B;
	} vi_Temp;

	ow_command(DS18_CMD_READ, pt_TempId, pb_Progmem);
	vi_Temp.B.L = ow_byte_rd();
	vi_Temp.B.H = ow_byte_rd();
	return vi_Temp.W;
} //DS18ReadMeas
