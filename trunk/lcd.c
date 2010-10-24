


#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "rc.h"
#include "fb.h"
#ifdef USE_ASM
#include "asm.h"
#endif

struct lcd lcd;

struct scan scan;

#define BG (scan.bg)
#define WND (scan.wnd)
#define BUF (scan.buf)
#define PRI (scan.pri)

#define PAL1 (scan.pal1)
#define PAL2 (scan.pal2)
#define PAL4 (scan.pal4)

#define VS (scan.vs) /* vissprites */
#define NS (scan.ns)

#define L (scan.l) /* line */
#define X (scan.x) /* screen position */
#define Y (scan.y)
#define S (scan.s) /* tilemap position */
#define T (scan.t)
#define U (scan.u) /* position within tile */
#define V (scan.v)
#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)

byte patpix[4096][8][8];
byte patdirty[1024];
byte anydirty;

static int sprsort = 1;
static int sprdebug;

#define DEF_PAL { 0x78f0f0, 0x58b8b8, 0x487878, 0x184848 }

static int dmg_pal[4][4] = { DEF_PAL, DEF_PAL, DEF_PAL, DEF_PAL };

rcvar_t lcd_exports[] =
{
	RCV_VECTOR("dmg_bgp", dmg_pal[0], 4),
	RCV_VECTOR("dmg_wndp", dmg_pal[1], 4),
	RCV_VECTOR("dmg_obp0", dmg_pal[2], 4),
	RCV_VECTOR("dmg_obp1", dmg_pal[3], 4),
	RCV_BOOL("sprsort", &sprsort),
	RCV_BOOL("sprdebug", &sprdebug),
	RCV_END
};

static byte *vdest;

#ifdef ALLOW_UNALIGNED_IO /* long long is ok since this is i386-only anyway? */
#define MEMCPY8(d, s) ((*(long long *)(d)) = (*(long long *)(s)))
#else
#define MEMCPY8(d, s) memcpy((d), (s), 8)
#endif




#ifndef ASM_PAT_UPDATEPIX
void pat_updatepix()
{
	int i, j, k;
	int a, c;
	byte *vram = lcd.vbank[0];
	
	if (!anydirty) return;
	for (i = 0; i < 1024; i++)
	{
		if (i == 384) i = 512;
		if (i == 896) break;
		if (!patdirty[i]) continue;
		patdirty[i] = 0;
		for (j = 0; j < 8; j++)
		{
			a = ((i<<4) | (j<<1));
			for (k = 0; k < 8; k++)
			{
				c = vram[a] & (1<<k) ? 1 : 0;
				c |= vram[a+1] & (1<<k) ? 2 : 0;
				patpix[i+1024][j][k] = c;
			}
			for (k = 0; k < 8; k++)
				patpix[i][j][k] =
					patpix[i+1024][j][7-k];
		}
		for (j = 0; j < 8; j++)
		{
			for (k = 0; k < 8; k++)
			{
				patpix[i+2048][j][k] =
					patpix[i][7-j][k];
				patpix[i+3072][j][k] =
					patpix[i+1024][7-j][k];
			}
		}
	}
	anydirty = 0;
}
#endif /* ASM_PAT_UPDATEPIX */



void tilebuf()
{
	int i, cnt;
	int base;
	byte *tilemap, *attrmap;
	int *tilebuf;
	int *wrap;
	static int wraptable[64] =
	{
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,-32
	};

	base = ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5) + S;
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = BG;
	wrap = wraptable + S;
	cnt = ((WX + 7) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *tilemap
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*tilemap))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*attrmap & 0x07) << 2);
				attrmap += *wrap + 1;
				tilemap += *(wrap++) + 1;
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++);
				tilemap += *(wrap++);
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*(tilemap++)));
				tilemap += *(wrap++);
			}
	}

	if (WX >= 160) return;
	
	base = ((R_LCDC&0x40)?0x1C00:0x1800) + (WT<<5);
	tilemap = lcd.vbank[0] + base;
	attrmap = lcd.vbank[1] + base;
	tilebuf = WND;
	cnt = ((160 - WX) >> 3) + 1;

	if (hw.cgb)
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = *(tilemap++)
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&7) << 2);
			}
		else
			for (i = cnt; i > 0; i--)
			{
				*(tilebuf++) = (256 + ((n8)*(tilemap++)))
					| (((int)*attrmap & 0x08) << 6)
					| (((int)*attrmap & 0x60) << 5);
				*(tilebuf++) = (((int)*(attrmap++)&7) << 2);
			}
	}
	else
	{
		if (R_LCDC & 0x10)
			for (i = cnt; i > 0; i--)
				*(tilebuf++) = *(tilemap++);
		else
			for (i = cnt; i > 0; i--)
				*(tilebuf++) = (256 + ((n8)*(tilemap++)));
	}
}


void bg_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;
	cnt = WX;
	tile = BG;
	dest = BUF;
	
	src = patpix[*(tile++)][V] + U;
	memcpy(dest, src, 8-U);
	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][V];
		MEMCPY8(dest, src);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*tile][V];
	while (cnt--)
		*(dest++) = *(src++);
}

void wnd_scan()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX >= 160) return;
	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;
	
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][WV];
		MEMCPY8(dest, src);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*tile][WV];
	while (cnt--)
		*(dest++) = *(src++);
}

void blendcpy(byte *dest, byte *src, byte b, int cnt)
{
	while (cnt--) *(dest++) = *(src++) | b;
}

static int priused(void *attr)
{
	un32 *a = attr;
	return (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);
}

void bg_scan_pri()
{
	int cnt, i;
	byte *src, *dest;

	if (WX <= 0) return;
	i = S;
	cnt = WX;
	dest = PRI;
	src = lcd.vbank[1] + ((R_LCDC&0x08)?0x1C00:0x1800) + (T<<5);

	if (!priused(src))
	{
		memset(dest, 0, cnt);
		return;
	}
	
	memset(dest, src[i++&31]&128, 8-U);
	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		memset(dest, src[i++&31]&128, 8);
		dest += 8;
		cnt -= 8;
	}
	memset(dest, src[i&31]&128, cnt);
}

void wnd_scan_pri()
{
	int cnt, i;
	byte *src, *dest;

	if (WX >= 160) return;
	i = 0;
	cnt = 160 - WX;
	dest = PRI + WX;
	src = lcd.vbank[1] + ((R_LCDC&0x40)?0x1C00:0x1800) + (WT<<5);
	
	if (!priused(src))
	{
		memset(dest, 0, cnt);
		return;
	}
	
	while (cnt >= 8)
	{
		memset(dest, src[i++]&128, 8);
		dest += 8;
		cnt -= 8;
	}
	memset(dest, src[i]&128, cnt);
}

void bg_scan_color()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX <= 0) return;
	cnt = WX;
	tile = BG;
	dest = BUF;
	
	src = patpix[*(tile++)][V] + U;
	blendcpy(dest, src, *(tile++), 8-U);
	dest += 8-U;
	cnt -= 8-U;
	if (cnt <= 0) return;
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][V];
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*(tile++)][V];
	blendcpy(dest, src, *(tile++), cnt);
}

void wnd_scan_color()
{
	int cnt;
	byte *src, *dest;
	int *tile;

	if (WX >= 160) return;
	cnt = 160 - WX;
	tile = WND;
	dest = BUF + WX;
	
	while (cnt >= 8)
	{
		src = patpix[*(tile++)][WV];
		blendcpy(dest, src, *(tile++), 8);
		dest += 8;
		cnt -= 8;
	}
	src = patpix[*(tile++)][WV];
	blendcpy(dest, src, *(tile++), cnt);
}

static void recolor(byte *buf, byte fill, int cnt)
{
	while (cnt--) *(buf++) |= fill;
}

void spr_count()
{
	int i;
	struct obj *o;
	
	NS = 0;
	if (!(R_LCDC & 0x02)) return;
	
	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		if (++NS == 10) break;
	}
}

void spr_enum()
{
	int i, j;
	struct obj *o;
	struct vissprite ts;
	int v, pat;

	NS = 0;
	if (!(R_LCDC & 0x02)) return;

	o = lcd.oam.obj;
	
	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		VS[NS].x = (int)o->x - 8;
		v = L - (int)o->y + 16;
		if (hw.cgb)
		{
			pat = o->pat | (((int)o->flags & 0x60) << 5)
				| (((int)o->flags & 0x08) << 6);
			VS[NS].pal = 32 + ((o->flags & 0x07) << 2);
		}
		else
		{
			pat = o->pat | (((int)o->flags & 0x60) << 5);
			VS[NS].pal = 32 + ((o->flags & 0x10) >> 2);
		}
		VS[NS].pri = (o->flags & 0x80) >> 7;
		if ((R_LCDC & 0x04))
		{
			pat &= ~1;
			if (v >= 8)
			{
				v -= 8;
				pat++;
			}
			if (o->flags & 0x40) pat ^= 1;
		}
		VS[NS].buf = patpix[pat][v];
		if (++NS == 10) break;
	}
	if (!sprsort || hw.cgb) return;
	for (i = 0; i < NS; i++)
	{
		for (j = i + 1; j < NS; j++)
		{
			if (VS[i].x > VS[j].x)
			{
				ts = VS[i];
				VS[i] = VS[j];
				VS[j] = ts;
			}
		}
	}
}

void spr_scan()
{
	int i, x;
	byte pal, b, ns = NS;
	byte *src, *dest, *bg, *pri;
	struct vissprite *vs;
	static byte bgdup[256];

	if (!ns) return;

	memcpy(bgdup, BUF, 256);
	vs = &VS[ns-1];
	
	for (; ns; ns--, vs--)
	{
		x = vs->x;
		if (x > 160) continue;
		if (x < -7) continue;
		if (x < 0)
		{
			src = vs->buf - x;
			dest = BUF;
			i = 8 + x;
		}
		else
		{
			src = vs->buf;
			dest = BUF + x;
			if (x > 152) i = 160 - x;
			else i = 8;
		}
		pal = vs->pal;
		if (vs->pri)
		{
			bg = bgdup + (dest - BUF);
			while (i--)
			{
				b = src[i];
				if (b && !(bg[i]&3)) dest[i] = pal|b;
			}
		}
		else if (hw.cgb)
		{
			bg = bgdup + (dest - BUF);
			pri = PRI + (dest - BUF);
			while (i--)
			{
				b = src[i];
				if (b && (!pri[i] || !(bg[i]&3)))
					dest[i] = pal|b;
			}
		}
		else while (i--) if (src[i]) dest[i] = pal|src[i];
	}
	if (sprdebug) for (i = 0; i < NS; i++) BUF[i<<1] = 36;
}

#ifndef ASM_REFRESH_1
void refresh_1(byte *dest, byte *pal)
{
	int i; byte *src = BUF-1;
	dest--; for (i = 160; i; i--) dest[i] = pal[src[i]];
}
#endif

#ifndef ASM_REFRESH_2
void refresh_2(un16 *dest, un16 *pal)
{
	int i; byte *src = BUF-1;
	dest--; for (i = 160; i; i--) dest[i] = pal[src[i]];
}
#endif

#ifndef ASM_REFRESH_3
void refresh_3(byte *dest, un32 *pal)
{
	int i; byte *src = BUF;
	un32 color;
	for (i = 160; i; i--)
	{
		color = pal[*(src++)];
		*(dest++) = color;
		*(dest++) = color>>8;
		*(dest++) = color>>16;
	}
}
#endif

#ifndef ASM_REFRESH_4
void refresh_4(un32 *dest, un32 *pal)
{
	int i; byte *src = BUF-1;
	dest--; for (i = 160; i; i--) dest[i] = pal[src[i]];
}
#endif









void lcd_begin()
{
	if (fb.indexed) pal_expire();
	vdest = fb.ptr + ((fb.w*fb.pelsize)>>1)
		- (80*fb.pelsize)
		+ ((fb.h>>1) - 72) * fb.pitch;
	WY = R_WY;
}

void lcd_refreshline()
{
	if (!(R_LCDC & 0x80))
	{
		if (fb.enabled) memset(vdest, 0, fb.pitch);
		vdest += fb.pitch;
		return;
	}
	
	pat_updatepix();

	L = R_LY;
	X = R_SCX;
	Y = (R_SCY + L) & 0xff;
	S = X >> 3;
	T = Y >> 3;
	U = X & 7;
	V = Y & 7;
	
	WX = R_WX - 7;
	if (WY>L || WY<0 || WY>143 || WX<-7 || WX>159 || !(R_LCDC&0x20))
		WX = 160;
	WT = (L - WY) >> 3;
	WV = (L - WY) & 7;

	spr_enum();
	tilebuf();
	if (hw.cgb)
	{
		bg_scan_color();
		wnd_scan_color();
		if (NS)
		{
			bg_scan_pri();
			wnd_scan_pri();
		}
	}
	else
	{
		bg_scan();
		wnd_scan();
		recolor(BUF+WX, 0x04, 160-WX);
	}
	spr_scan();

	if (fb.enabled)
	{
		if (fb.dirty) memset(fb.ptr, 0, fb.pitch * fb.h);
		fb.dirty = 0;
		switch (fb.pelsize)
		{
		case 1:
			refresh_1(vdest, PAL1);
			break;
		case 2:
			refresh_2((void*)vdest, PAL2);
			break;
		case 3:
			refresh_3((void*)vdest, PAL4);
			break;
		case 4:
			refresh_4((void*)vdest, PAL4);
			break;
		}
	}
	
	vdest += fb.pitch;
}







static void updatepalette(int i)
{
	int c, r, g, b;

	c = (lcd.pal[i<<1] | ((int)lcd.pal[(i<<1)|1] << 8)) & 0x7FFF;
	r = (c & 0x001F) << 3;
	g = (c & 0x03E0) >> 2;
	b = (c & 0x7C00) >> 7;
	r |= (r >> 5);
	g |= (g >> 5);
	b |= (b >> 5);
	
	if (fb.indexed)
	{
		pal_release(PAL1[i]);
		PAL1[i] = pal_getcolor(c, r, g, b);
		return;
	}

	r = (r >> fb.cc[0].r) << fb.cc[0].l;
	g = (g >> fb.cc[1].r) << fb.cc[1].l;
	b = (b >> fb.cc[2].r) << fb.cc[2].l;
	c = r|g|b;
	PAL1[i] = PAL2[i] = PAL4[i] = c;
}

void pal_write(int i, byte b)
{
	if (lcd.pal[i] == b) return;
	lcd.pal[i] = b;
	updatepalette(i>>1);
}

void pal_write_dmg(int i, int mapnum, byte d)
{
	int j;
	int *cmap = dmg_pal[mapnum];
	int c, r, g, b;

	if (hw.cgb) return;

	for (j = 0; j < 8; j += 2)
	{
		c = cmap[(d >> j) & 3];
		r = (c & 0xf8) >> 3;
		g = (c & 0xf800) >> 6;
		b = (c & 0xf80000) >> 9;
		c = r|g|b;
		/* FIXME - handle directly without faking cgb */
		pal_write(i+j, c & 0xff);
		pal_write(i+j+1, c >> 8);
	}
}

void vram_write(addr a, byte b)
{
	lcd.vbank[R_VBK][a] = b;
	if (a >= 0x1800) return;
	patdirty[(R_VBK<<9)+(a>>4)] = 1;
	anydirty = 1;
}

void vram_dirty()
{
	anydirty = 1;
	memset(patdirty, 1, sizeof patdirty);
}

void pal_dirty()
{
	int i;
	if (hw.cgb)
		for (i = 0; i < 64; i++)
			updatepalette(i);
	else
	{
		pal_write_dmg(0, 0, R_BGP);
		pal_write_dmg(8, 1, R_BGP);
		pal_write_dmg(64, 2, R_OBP0);
		pal_write_dmg(72, 3, R_OBP1);
	}
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);
	lcd_begin();
	vram_dirty();
	pal_dirty();
}















