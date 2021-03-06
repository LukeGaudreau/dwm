/* See LICENSE file for copyright and license details. */
#include "push.c"
#include <X11/XF86keysym.h>

/* appearance */
static const char *fonts[] = {
	"SF Pro Display:size=11",
  "icomoon:size=11"
};
static const char dmenufont[]       = "Iosevka:size=11";
static const char normbordercolor[] = "#444444";
static const char normbgcolor[]     = "#000000";
static const char normfgcolor[]     = "#e3e0d5";
static const char selbordercolor[]  = "#bdb3a0";
static const char selbgcolor[]      = "#333333";
static const char selfgcolor[]      = "#e3e0d5";
static const unsigned int borderpx  = 2;      /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappx     = 20;       /* gap pixel between windows */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char clock_fmt[]       = "%a, %b %e %l:%M %p";

/* tagging */
static const char *tags[] = { "", "", "", "", "", "", "", "", "" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance        title       tags mask     iscentered isfloating   monitor */
	{ "Firefox",  "Navigator",    NULL,       1 << 1,       0,         0,           -1 },
	{ "Firefox",  "Devtools",     NULL,       1 << 1,       0,         0,            1 },
	{ "qutebrowser",  NULL,       NULL,       1 << 1,       0,         0,            1 },
	{ "Chromium", NULL,           NULL,       1 << 1,       0,         0,           -1 },
	{ NULL,       NULL,           "TODO",          0,       1,         1,           -1 },
	{ NULL,       NULL,           "Agenda",        0,       1,         1,           -1 },
	{ NULL,       NULL,           "- Org",    1 << 3,       0,         0,           -1 },
	{ NULL,       NULL,           "- Code",   1 << 2,       0,         0,           -1 },
	{ NULL,       NULL,           "- Spacemacs",   1 << 2,       0,         0,      -1 },
	{ "Slack",    NULL,           NULL,       1 << 4,       0,         0,            1 },
	{ "Spotify",  NULL,           NULL,       1 << 6,       0,         0,            1 },
	{ "Remmina",  NULL,           NULL,       1 << 7,       0,         0,            1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "",      tile },    /* first entry is default */
	{ "",      NULL },    /* no layout function means floating behavior */
	{ "",      monocle },
	{ "TTT",      bstack },
	{ "===",      bstackhoriz },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *appmenucmd[] = {"rofi", "-show", "drun" };
static const char *dmenucmd[] = {"rofi", "-show", "run" };
static const char *sshmenucmd[] = {"rofi", "-show", "ssh" };
static const char *winmenucmd[] = {"rofi", "-show", "window" };
static const char *passcmd[] = { "pass_launcher" };
static const char *displaycmd[] = { "display_menu" };
static const char *termcmd[]  = { "st", NULL };
static const char *capturecmd[] = {"emacs-org-capture", "org-protocol://capture?template=t"};

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = passcmd } },
	{ MODKEY|ShiftMask,             XK_p,      spawn,          {.v = displaycmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } } ,
	{ Mod1Mask|ShiftMask,           XK_Return, spawn,          {.v = sshmenucmd } } ,
	{ MODKEY|ShiftMask,             XK_slash,  spawn,          SHCMD("emacs-agenda-popup")},
	{ MODKEY|ShiftMask,             XK_e,      spawn,          SHCMD("emacs-code") },
	{ MODKEY|ShiftMask,             XK_o,      spawn,          SHCMD("emacs-org") },
  { Mod1Mask,                     XK_space,  spawn,          {.v = capturecmd } },
  { Mod1Mask,                     XK_Tab,    spawn,          {.v = winmenucmd } },
	{ MODKEY|ShiftMask,             XK_l,      spawn,          SHCMD("slock") },
	{ MODKEY,                       XK_s,      spawn,          SHCMD("flameshot gui") },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          SHCMD("flameshot full -c -p ~/Pictures/Screenshots") },
	{ MODKEY|ShiftMask,             XK_v,      spawn,          SHCMD("vpn toggle") },
	{ 0,             XF86XK_MonBrightnessDown, spawn,          SHCMD("brightnessctl -q s 10%-") },
	{ 0,             XF86XK_MonBrightnessUp,   spawn,          SHCMD("brightnessctl -q s +10%") },
	{ ShiftMask,     XF86XK_MonBrightnessUp,   spawn,          SHCMD("brightnessctl -q s 100%") },
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
	{ MODKEY,                       XK_space,  spawn,          {.v = appmenucmd } },
	{ MODKEY|ShiftMask,             XK_space,  spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_Left,   focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_Right,  focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_Left,   tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Right,  tagmon,         {.i = +1 } },
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
/* click can be ClkLtSymbol, ClkStatusText, ClkClock, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

