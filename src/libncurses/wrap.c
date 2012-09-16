#include <curses.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t __darwin_acs_map[256];

WINDOW* __darwin_initscr(void)
{
	WINDOW* win = initscr();

	for (int i = 0; i < 256; i++)
		__darwin_acs_map[i] = acs_map[i];

	return win;
}

int __darwin_wborder (WINDOW * w,uint32_t c1,uint32_t c2,uint32_t c3,uint32_t c4,uint32_t c5,uint32_t c6,uint32_t c7,uint32_t c8)
{
	return wborder(w,c1,c2,c3,c4,c5,c6,c7,c8);
}

int __darwin_pechochar (WINDOW * w, const uint32_t c)
{
	return pechochar(w, c);
}

int __darwin_slk_attroff (const uint32_t c)
{
	return slk_attroff(c);
}

int __darwin_slk_attron (const uint32_t c)
{
	return slk_attron(c);
}

int __darwin_slk_attrset (const uint32_t c)
{
	return slk_attrset(c);
}

uint32_t __darwin_termattrs (void)
{
	return termattrs();
}

int __darwin_vidattr (uint32_t c)
{
	return vidattr(c);
}

int __darwin_vidputs (uint32_t, NCURSES_OUTC);

int __darwin_waddch (WINDOW *w, const uint32_t c)
{
	return waddch(w, c);
}

int __darwin_waddchnstr (WINDOW * w,const uint32_t * c,int n)
{
	chtype* nc = malloc(sizeof(chtype)*n);
	for (int i = 0; i < n; i++)
		nc[i] = c[i];
	int rv = waddchnstr(w, nc, n);
	free(nc);
	return rv;
}

int __darwin_wbkgd (WINDOW * w, uint32_t c)
{
	return wbkgd(w, c);
}

void __darwin_wbkgdset (WINDOW *w,uint32_t c)
{
	wbkgdset(w, c);
}

int __darwin_wechochar (WINDOW *w, const uint32_t c)
{
	return wechochar(w, c);
}

int __darwin_whline (WINDOW *w, uint32_t c, int n)
{
	return whline(w, c, n);
}

uint32_t __darwin_winch (WINDOW *w)
{
	return winch(w);
}

int __darwin_winchnstr (WINDOW *w, uint32_t *c, int n)
{
	chtype* nc = malloc(sizeof(chtype)*n);
	int rv = winchnstr(w, nc, n);
	for (int i = 0; i < n; i++)
		c[i] = nc[i];
    free(nc);
    return rv;
}

int __darwin_winsch (WINDOW *w, uint32_t c)
{
	return winsch(w, c);
}

int __darwin_wvline (WINDOW *w,uint32_t c,int n)
{
	return wvline(w,c,n);
}

int __darwin_slk_attroff_sp (SCREEN*, const uint32_t);
int __darwin_slk_attron_sp (SCREEN*, const uint32_t);
int __darwin_slk_attrset_sp (SCREEN*, const uint32_t);
uint32_t __darwin_termattrs_sp (SCREEN*);
int __darwin_vidattr_sp (SCREEN*, uint32_t);
int __darwin_vidputs_sp (SCREEN*, uint32_t, NCURSES_SP_OUTC);

uint32_t __darwin_slk_attr (void)
{
	return slk_attr();
}

int __darwin_slk_attr_set (const uint32_t a,short s,void* p)
{
	return slk_attr_set(a,s,p);
}
int __darwin_wattr_on (WINDOW *w, uint32_t a, void *p)
{
	return wattr_on(w,a,p);
}

int __darwin_wattr_off (WINDOW *w, uint32_t a, void *p)
{
	return wattr_off(w,a,p);
}

int __darwin_wchgat (WINDOW *w, int i, uint32_t a, short s, const void *p)
{
	return wchgat(w, i, a, s, p);
}

uint32_t __darwin_slk_attr_sp (SCREEN*);
int __darwin_slk_attr_set_sp (SCREEN*, const uint32_t, short, void*);


