#include "bufor.h"

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "Charset.h"
#include "TempUtil.h"

uint8_t gi_EEBuforTempMINGrzalkiDzien EEMEM = 25;
uint8_t gi_EEBuforTempMAXGrzalkiDzien EEMEM = 50;

uint8_t gi_EEBuforTempMINGrzalkiNoc EEMEM = 50;
uint8_t gi_EEBuforTempMAXGrzalkiNoc EEMEM = 85;

uint8_t gi_EEBuforDzienStartH EEMEM = 6;
uint8_t gi_EEBuforDzienEndH EEMEM = 23;

void BuforGrzej(uint8_t pb_grzej) {
	if (pb_grzej) {
		GRZALKA_PORT |= (GRZALKA_1 | GRZALKA_2 | GRZALKA_3);
		gb_BuforStatus = BUFOR_GRZEJE;
	} else {
		GRZALKA_PORT &= 0xFF & !(GRZALKA_1 | GRZALKA_2 | GRZALKA_3);
		gb_BuforStatus = BUFOR_NIE_GRZEJE;
	}
}

void BuforInit(void) {
	GRZALKA_DDR = 0xFF;
	GRZALKA_PORT &= 0xFF & !(GRZALKA_1 | GRZALKA_2 | GRZALKA_3);
	gi_BuforTempMINGrzalkiDzien = eeprom_read_byte((uint8_t *) &gi_EEBuforTempMINGrzalkiDzien);
	if (gi_BuforTempMINGrzalkiDzien == 255) {
		gi_BuforTempMINGrzalkiDzien = 25;
	}
	gi_BuforTempMAXGrzalkiDzien = eeprom_read_byte((uint8_t *) &gi_EEBuforTempMAXGrzalkiDzien);
	if (gi_BuforTempMAXGrzalkiDzien == 255) {
		gi_BuforTempMAXGrzalkiDzien = 50;
	}

	gi_BuforTempMINGrzalkiNoc = eeprom_read_byte((uint8_t *) &gi_EEBuforTempMINGrzalkiNoc);
	if (gi_BuforTempMINGrzalkiNoc == 255) {
		gi_BuforTempMINGrzalkiNoc = 50;
	}

	gi_BuforTempMAXGrzalkiNoc = eeprom_read_byte((uint8_t *) &gi_EEBuforTempMAXGrzalkiNoc);
	if (gi_BuforTempMAXGrzalkiNoc == 255) {
		gi_BuforTempMAXGrzalkiNoc = 85;
	}

	gi_BuforDzienStartH = eeprom_read_byte((uint8_t *) &gi_EEBuforDzienStartH);
	if (gi_BuforDzienStartH == 255) {
		gi_BuforDzienStartH = 6;
	}

	gi_BuforDzienEndH = eeprom_read_byte((uint8_t *) &gi_EEBuforDzienEndH);
	if (gi_BuforDzienEndH == 255) {
		gi_BuforDzienEndH = 23;
	}

}

void BuforChange(uint8_t pi_Y, uint8_t pi_What //MIN_d MAX_d MIN_N MAX_N
		, int8_t pi_Value) {
	if (pi_Value) { //zmiana
		switch (pi_What) {
		case MIND:
			gi_BuforTempMINGrzalkiDzien += pi_Value;
			eeprom_write_byte(&gi_EEBuforTempMINGrzalkiDzien, gi_BuforTempMINGrzalkiDzien);
			break;
		case MAXD:
			gi_BuforTempMAXGrzalkiDzien += pi_Value;
			eeprom_write_byte(&gi_EEBuforTempMAXGrzalkiDzien, gi_BuforTempMAXGrzalkiDzien);
			break;
		case MINN:
			gi_BuforTempMINGrzalkiNoc += pi_Value;
			eeprom_write_byte(&gi_EEBuforTempMINGrzalkiNoc, gi_BuforTempMINGrzalkiNoc);
			break;
		case MAXN:
			gi_BuforTempMAXGrzalkiNoc += pi_Value;
			eeprom_write_byte(&gi_EEBuforTempMAXGrzalkiNoc, gi_BuforTempMAXGrzalkiNoc);
			break;
		case DSTART:
			gi_BuforDzienStartH += pi_Value;
			eeprom_write_byte(&gi_EEBuforDzienStartH, gi_BuforDzienStartH);
			break;
		case DEND:
			gi_BuforDzienEndH += pi_Value;
			eeprom_write_byte(&gi_EEBuforDzienEndH, gi_BuforDzienEndH);
			break;

		}
	} //wyswietlenie
		CSSet(CS_Z6p);
	switch (pi_What) {
	case MIND:
		CSPrintXYu8AV_p(LCD_W/2, pi_Y+6, PSTR("MIND:"), ALIGN_CENTER, LCD_W);
		CSPrintXYu8AV(LCD_W/2, pi_Y+12, CSInt2V(gi_BuforTempMINGrzalkiDzien,2,0), ALIGN_CENTER, LCD_W);
		break;
	case MAXD:
		CSPrintXYu8AV_p(LCD_W/2, pi_Y+6, PSTR("MAXD:"), ALIGN_CENTER, LCD_W);
		CSPrintXYu8AV(LCD_W/2, pi_Y+12, CSInt2V(gi_BuforTempMAXGrzalkiDzien,2,0), ALIGN_CENTER, LCD_W);
		break;
	case MINN:
		CSPrintXYu8AV_p(LCD_W/2, pi_Y+6, PSTR("MINN:"), ALIGN_CENTER, LCD_W);
		CSPrintXYu8AV(LCD_W/2, pi_Y+12, CSInt2V(gi_BuforTempMINGrzalkiNoc,2,0), ALIGN_CENTER, LCD_W);
		break;
	case MAXN:
		CSPrintXYu8AV_p(LCD_W/2, pi_Y+6, PSTR("MIND:"), ALIGN_CENTER, LCD_W);
		CSPrintXYu8AV(LCD_W/2, pi_Y+12, CSInt2V(gi_BuforTempMAXGrzalkiNoc,2,0), ALIGN_CENTER, LCD_W);
		break;
	case DSTART:
		CSPrintXYu8AV_p(LCD_W/2, pi_Y+6, PSTR("Pierwsza godzina dnia:"), ALIGN_CENTER, LCD_W);
		CSPrintXYu8AV(LCD_W/2, pi_Y+12, CSInt2V(gi_BuforDzienStartH,2,0), ALIGN_CENTER, LCD_W);
		break;
	case DEND:
		CSPrintXYu8AV_p(LCD_W/2, pi_Y+6, PSTR("Ostatnia godzina dnia:"), ALIGN_CENTER, LCD_W);
		CSPrintXYu8AV(LCD_W/2, pi_Y+12, CSInt2V(gi_BuforDzienEndH,2,0), ALIGN_CENTER, LCD_W);
		break;

	}
}

int8_t BuforSprawdz() {
	float vf_temp = TempGetF(0,TEMP_TICK), vf_tMin;
	uint8_t vf_tMax;
	if (TimeIsBetween(gi_BuforDzienStartH, gi_BuforDzienEndH)) { //dzieñ
		vf_tMax = gi_BuforTempMAXGrzalkiDzien;
		vf_tMin = gi_BuforTempMINGrzalkiDzien;
		gb_BuforDzien = 1;
	} else {
		vf_tMax = gi_BuforTempMAXGrzalkiNoc;
		vf_tMin = gi_BuforTempMINGrzalkiNoc;
		gb_BuforDzien = 0;
	}

	switch (gb_BuforStatus) {
	case BUFOR_GRZEJE:
		if (vf_temp >= vf_tMax) {
			BuforGrzej(0); //grzeje ale przekroczyl max to wylacz
		}
		break;

	case BUFOR_NIE_GRZEJE:
		if (vf_temp <= vf_tMin) {
			BuforGrzej(1); //nie grzeje i wychlodzil sie to wlacz
		}
		break;
	}
	return vf_tMax;
}

