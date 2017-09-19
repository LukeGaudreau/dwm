/* See LICENSE file for copyright and license details. */
#include "push.c"
#include <X11/XF86keysym.h>

/* appearance */
static const char *fonts[] = {
	"Ubuntu:size=11",
	"FontAwesome:size=12"
};
static const char dmenufont[]       = "Ubuntu:size=11";
#define NUMCOLORS         9
static const char colors[NUMCOLORS][MAXCOLORS][9] = {
  // border   foreground background
  { "#222222", "#687080", "#222222" }, // 0 = normal
  { "#589CD3", "#E0E0E0", "#222222" }, // 1 = selected
  { "#282C34", "#FF6C6B", "#222222" }, // 2 = red
  { "#282C34", "#98BE65", "#222222" }, // 3 = green
  { "#282C34", "#914E89", "#222222" }, // 4 = yellow
  { "#282C34", "#589CD3", "#222222" }, // 5 = blue
  { "#282C34", "#0088CC", "#222222" }, // 6 = cyan
  { "#282C34", "#7E62B3", "#222222" }, // 7 = magenta
  { "#282C34", "#899CA1", "#222222" }, // 8 = grey
};
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappx     = 10;       /* gap pixel between windows */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */

/* tagging */
static const char *tags[] = { "base", "web", "org", "mail", "vm", "office", "art", "play", "test" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
  { "Firefox",  NULL,       NULL,       1 << 1,       0,           -1 },
  { "Chromium", NULL,       NULL,       1 << 1,       0,           -1 },
  { NULL,      "emacs-org", NULL,       1 << 2,       0,           -1 },
  { NULL,      "emacs-twitter", NULL,   1 << 1,       0,           -1 },
  { NULL,      "emacs-elfeed", NULL,    1 << 1,       0,           -1 },
  { NULL,      "emacs-notmuch", NULL,   1 << 3,       0,           -1 },
  { "spotify",  NULL,       NULL,       1 << 7,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "TTT",      bstack },
	{ "===",      bstackhoriz },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-p", "run:", "-m", dmenumon, "-fn", dmenufont, "-nb", colors[0][2], "-nf", colors[0][1], "-sb", colors[1][2], "-sf", colors[1][1], NULL };
static const char *passcmd[] = { "passmenu", "-p", "pass:", "-m", dmenumon, "-fn", dmenufont, "-nb", colors[0][2], "-nf", colors[0][1], "-sb", colors[1][2], "-sf", colors[1][1], NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *mailcmd[]  = { "termite", "-e", "mutt", "-t", "mutt", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = passcmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_m,      spawn,          SHCMD("emacs-notmuch")},
	{ MODKEY|ShiftMask,             XK_e,      spawn,          SHCMD("emacsclient -c -n -e '(switch-to-buffer nil)'") },
	{ MODKEY|ShiftMask,             XK_w,      spawn,          SHCMD("firefox") },
	{ MODKEY|ShiftMask,             XK_o,      spawn,          SHCMD("emacs-org") },
  { MODKEY|ShiftMask,             XK_f,      spawn,          SHCMD("emacs-elfeed") },
	{ MODKEY|ShiftMask,             XK_t,      spawn,          SHCMD("emacs-twitter") },
	{ MODKEY|ShiftMask,             XK_l,      spawn,          SHCMD("slock") },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          SHCMD("screenshot-selection") },
	{ 0,             XF86XK_AudioLowerVolume,  spawn,          SHCMD("pamixer -d 5") },
	{ 0,             XF86XK_AudioMute,         spawn,          SHCMD("pamixer -t") },
	{ 0,             XF86XK_AudioRaiseVolume,  spawn,          SHCMD("pamixer -i 5") },
	{ 0,             XF86XK_AudioPlay,         spawn,          SHCMD("mpc toggle") },
	{ 0,             XF86XK_AudioNext,         spawn,          SHCMD("mpc next") },
	{ 0,             XF86XK_AudioPrev,         spawn,          SHCMD("mpc prev") },
	{ 0,             XF86XK_AudioStop,         spawn,          SHCMD("mpc stop") },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,      pushdown,       {0} },
	{ MODKEY|ShiftMask,             XK_k,      pushup,         {0} },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY|ShiftMask,             XK_f,      togglefloating, {0} }, 
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_o,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY,                       XK_slash,  setlayout,      {0} },
	{ MODKEY,                       XK_space,  spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

