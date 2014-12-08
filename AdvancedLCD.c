#include "AdvancedLCD.h"

#include <avr/pgmspace.h>

#include "BasicLCD.h"

//void ADVPrintPix(int16_t pi_X, int16_t pi_Y, uint8_t pb_Fill //1:zapal 0:zgaœ
//		) {
//	GLCD_SetPixel(pi_X, pi_Y, pb_Fill && !gb_LCDNegMode);
//}

//*********************************
//zapal/zgaœ pojedynczy piksel
void ADVPrintPix(
  int16_t pi_X
 ,int16_t pi_Y
 ,uint8_t pb_Fill //1:zapal 0:zgaœ
) {
	uint8_t vi_Byte;

	LCDSetXY8(pi_X,pi_Y>>3);
	LCDReadRow(&vi_Byte,1);
	if (pb_Fill && !gb_LCDNegMode)
		vi_Byte |= _BV(pi_Y&7);
	else
		vi_Byte &= ~_BV(pi_Y&7);
	LCDSetXY8(pi_X,pi_Y>>3);
	LCDWriteData(vi_Byte);
} //ADVPrintPix
//*********************************
//drukuj/czyœæ kwadrat - dziala
//void ADVPrintRect(int16_t pi_X      //x
//		, int16_t pi_Y      //y
//		, uint8_t pi_Width  //szerokoœæ
//		, uint8_t pi_Height //wysokoœæ
//		, uint8_t pb_Fill   //1:wype³nij 0:wyczyœæ
//		) {
//for(int16_t i=pi_X; i< pi_X+pi_Width;i++){
//	for(int16_t j=pi_Y;j<pi_Y+pi_Height;j++){
//		ADVPrintPix(i,j,pb_Fill);
//	}
//}
//} //ADVPrintRect

//*********************************
//drukuj/czyœæ kwadrat
void ADVPrintRect(int16_t pi_X      //x
		, int16_t pi_Y      //y
		, uint8_t pi_Width  //szerokoœæ
		, uint8_t pi_Height //wysokoœæ
		, uint8_t pb_Fill   //1:wype³nij 0:wyczyœæ
		) {
	uint8_t vi_Mask, vi_Idx;
	int8_t vi_LastY8, vi_Y8;

	//oblicz pierwsz¹ maskê
	vi_Idx = pi_Y & 7;
	vi_Mask = 0xFF << vi_Idx;
	vi_LastY8 = (pi_Y + pi_Height - 1) >> 3;

	for (vi_Y8 = pi_Y >> 3; vi_Y8 <= vi_LastY8; vi_Y8++) {
		//oblicz ostatni¹ maskê
		if (vi_Y8 == vi_LastY8) {
			vi_Idx = 7 - ((pi_Y + pi_Height - 1) & 7);
			vi_Mask &= 0xFF >> vi_Idx;
		}

		LCDSetXY8(pi_X, vi_Y8);
		LCDFillClrRow(pb_Fill, pi_Width, vi_Mask);

		vi_Mask = 0xFF;
	}

} //ADVPrintRect

//*********************************
//drukuj ca³y obrazek wraz z marginesami
void ADVPrintImg(int16_t pi_X, int16_t pi_Y, uint8_t pi_SpaceXBefore,
		uint8_t pi_SpaceXAfter, uint8_t pi_Height, uint8_t *pt_Image) {

	if (!pt_Image || pi_X >= LCD_W || pi_Y > LCD_H)
		return;

	uint8_t vi_ImgW, vi_ImgH, vi_ImgYOffs;
	uint8_t vi_MoveBits, vi_Mask;
	int8_t vi_Y8, vi_FirstY8, vi_LastY8, vi_ImgFirstY8, vi_ImgLastY8;
	uint8_t *vi_Address, *vv_Data1, *vv_Data2;
	uint8_t vi_Idx;

	vi_ImgW = ADV_ImgWidth(pt_Image);
	vi_ImgH = ADV_ImgHeight(pt_Image);
	vi_ImgYOffs = ADV_ImgYOffs(pt_Image);
	if (pi_Height < vi_ImgH + vi_ImgYOffs) {
		pi_Y += vi_ImgYOffs;
		vi_ImgYOffs = 0;
		pi_Height = vi_ImgH;
	}
	vi_MoveBits = (pi_Y + vi_ImgYOffs) & 7;
	vi_Mask = 0xFF << (pi_Y & 7);
	vi_FirstY8 = pi_Y >> 3;
	vi_LastY8 = (pi_Y + pi_Height - 1) >> 3;
	vi_ImgFirstY8 = (pi_Y + vi_ImgYOffs) >> 3;
	vi_ImgLastY8 = vi_ImgFirstY8 + ((vi_ImgH - 1) >> 3);
	vi_Address = ADV_ImgData(pt_Image);
	vv_Data1 = 0;
	vv_Data2 = 0;

	for (vi_Y8 = vi_FirstY8; vi_Y8 <= vi_LastY8; vi_Y8++) {
		//oblicz tekst do wys³ania uwzglêdniaj¹c przesuwanie bitów

		//HGFEDCBA (0)
		//87654321 (1)
		//Y:3 H:16
		//Y8:0 EDCBA000 (       0<<3)
		//Y8:1 54321HGF (0>>5 | 1<<3)
		//Y8:2 00000876 (1>>5       )

		if (vi_MoveBits)
			vv_Data1 = vv_Data2;      //pobierz z Y-1
		if (vi_Y8 >= vi_ImgFirstY8 && vi_Y8 <= vi_ImgLastY8) {
			vv_Data2 = vi_Address; //pobierz z Y
			vi_Address += vi_ImgW;
		} else
			vv_Data2 = 0;
		//oblicz ostatni¹ maskê
		if (vi_Y8 == vi_LastY8) {
			vi_Idx = 7 - ((pi_Y + pi_Height - 1) & 7);
			vi_Mask &= 0xFF >> vi_Idx;
		}
		LCDSetXY8(pi_X, vi_Y8);
		LCDWriteRow_p(vv_Data1, vv_Data2, vi_MoveBits, vi_ImgW, pi_SpaceXBefore,
				pi_SpaceXAfter, vi_Mask);
		vi_Mask = 0xFF;
	}

} //ADVPrintImg

//*********************************
//drukuj obrazek w dowolnym miejscu; wydruk nie zmienia pozycji kursora
void ADVPrintImgPart(int16_t pi_X           //x
		, int16_t pi_Y           //y
		, uint8_t *pt_Image       //obrazek do wydrukowania
		, uint8_t pi_FirstXIdx   //pierwszy indeks X z obrazka
		, uint8_t pi_Width       //szerokoœæ wydruku
		) {

	if (!pt_Image || pi_X >= LCD_W || pi_Y > LCD_H)
		return;

	uint8_t vi_Width, vi_Height;
	uint8_t vi_Bytes;
	uint8_t vi_MoveBits;
	uint8_t vi_Mask;
	int8_t vi_Y8, vi_FirstY8, vi_LastY8;
	uint8_t *vi_Address, *vv_Data1, *vv_Data2;
	uint8_t vi_Idx;
	uint8_t vi_ImgY;

	vi_Width = ADV_ImgWidth(pt_Image);
	vi_Height = ADV_ImgHeight(pt_Image);
	pi_Y += ADV_ImgYOffs(pt_Image);
	vi_Bytes = pi_Width;
	vi_MoveBits = pi_Y & 7;
	vi_Mask = 0xFF << vi_MoveBits;
	vi_FirstY8 = pi_Y >> 3;
	vi_LastY8 = (pi_Y + vi_Height - 1) >> 3;
	vi_Address = ADV_ImgData(pt_Image)+pi_FirstXIdx;
	vi_ImgY = 0;
	vv_Data2 = 0;

	for (vi_Y8 = vi_FirstY8; vi_Y8 <= vi_LastY8; vi_Y8++) {
		//oblicz tekst do wys³ania uwzglêdniaj¹c przesuwanie bitów

		//HGFEDCBA (0)
		//87654321 (1)
		//Y:3 H:16
		//Y8:0 EDCBA000 (       0<<3)
		//Y8:1 54321HGF (0>>5 | 1<<3)
		//Y8:2 00000876 (1>>5       )

		vv_Data1 = vv_Data2;                          //pobierz z Y-1
		vv_Data2 = vi_ImgY < vi_Width ? vi_Address : 0; //pobierz z Y
		//oblicz ostatni¹ maskê
		if (vi_Y8 == vi_LastY8) {
			vi_Idx = 7 - ((pi_Y + vi_Height - 1) & 7);
			vi_Mask &= 0xFF >> vi_Idx;
		}
		LCDSetXY8(pi_X, vi_Y8);
		LCDWriteRow_p(vv_Data1, vv_Data2, vi_MoveBits, vi_Bytes, 0, 0, vi_Mask);
		vi_Address += vi_Width;
		vi_ImgY++;
		vi_Mask = 0xFF;
	}

} //ADVPrintImgPart
