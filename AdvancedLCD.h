#ifndef	ADVANCEDLCD_H
#define ADVANCEDLCD_H 1


#define ADV_ImgWidth(pv_Img) (pgm_read_byte(&pv_Img[0]))
#define ADV_ImgHeight(pv_Img) (pgm_read_byte(&pv_Img[1]))
#define ADV_ImgYOffs(pv_Img) (pgm_read_byte(&pv_Img[2])>>4)
#define ADV_ImgData(pv_Img) (&pv_Img[3])

#include <inttypes.h>

//zapal/zga� pojedynczy piksel
void ADVPrintPix(
  int16_t pi_X
 ,int16_t pi_Y
 ,uint8_t pb_Fill //1:zapal 0:zga�
);

//drukuj/czy�� kwadrat
void ADVPrintRect(
	int16_t pi_X      //x
 ,int16_t pi_Y      //y
 ,uint8_t pi_Width  //szeroko��
 ,uint8_t pi_Height //wysoko��
 ,uint8_t pb_Fill   //1:wype�nij 0:wyczy��
);

//drukuj ca�y obrazek wraz z marginesami
void ADVPrintImg(
  int16_t  pi_X
 ,int16_t  pi_Y
 ,uint8_t  pi_SpaceXBefore
 ,uint8_t  pi_SpaceXAfter
 ,uint8_t  pi_SpaceYAfter
 ,uint8_t *pt_Image
);

//drukuj cz�� obrazka (bez margines�w)
void ADVPrintImgPart(
  int16_t pi_X           //x
 ,int16_t pi_Y           //y
 ,uint8_t *pt_Image       //obrazek do wydrukowania
 ,uint8_t pi_FirstXIdx   //pierwszy indeks X z obrazka
 ,uint8_t pi_Width       //szeroko�� wydruku
);

#endif
