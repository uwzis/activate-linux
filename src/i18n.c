#include "i18n.h"
#include "options.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef GDI
  #include <windows.h>
  #ifndef LCIDToLocaleName
    WINBASEAPI int WINAPI LCIDToLocaleName(LCID Locale, LPWSTR lpName, int cchName, DWORD dwFlags);
  #endif
#endif


// Compare 5 fist chars from strings
#define match_str(match, with) (match && with && (strncmp(match, with, 5) == 0))
// Length of array
#define length(array) (sizeof(array) / sizeof(array[0]))


i18n_info_soup langs[] = {
// You are welcome to add your language here!
// You may use \n in subtitle to get new line. There is example with meaning of all strings:
// {"language_REGION", {"Title text prefix", "Title text suffix", "Subtitle text prefix", "Subtitle text suffix"},
//   {"Diss title prefix", "Diss title suffix", "Subtitle with Microsoft diss"}},

// English is default language, so it has to be first in the list
  {"en_US", {"Activate ", "", "Go to Settings to activate ", "."},
    {"No need to activate ", "", "We're not as annoying as Microsoft"}},
  {"fr_FR", {"Activer ", "", "Accédez aux paramètres pour activer ", "."},
    {NULL, NULL, NULL}},
  {"it_IT", {"Attiva ", "", "Passa a Impostazioni per attivare ", "."},
    {NULL, NULL, NULL}},
  {"ja_JP", {"", "のライセンス認証", "設定を開き、", "のライセンス認証を行ってください"},
    {NULL, NULL, NULL}},
  {"nl_NL", {"Activeren ", "", "Gaan naar instellingen om te activeren ", "."},
    {NULL, NULL, NULL}},
  {"ru_RU", {"Активация ", "", "Чтобы активировать ", ",\nперейдите в раздел \"Параметры\"."},
    {"Активировать ", " не надо", "Мы не так назойливы, как Microsoft"}},
  {"zh_CN", {"激活 ", "", "转到“设置”以激活 ", "。"},
    {NULL, NULL, NULL}},
  {"zh_TW", {"啟用 ", "", "移至[設定]以啟用 ", "。"},
    {NULL, NULL, NULL}},
  {"zh_HK", {"啟用 ", "", "移至[設定]以啟用 ", "。"},
    {NULL, NULL, NULL}},
};


#if defined(__APPLE__) || defined(__MACH__)
  #define DEFAULT_PRESET 0
#elif defined(__FreeBSD__) || defined(__NetBSD__) \
    || defined(__OpenBSD__) || defined(__DragonFly__) \
    || defined(__bsdi__)
  #define DEFAULT_PRESET 1
#elif defined(__linux__)
  #define DEFAULT_PRESET 2
#elif defined(__gnu_hurd__) || defined(__GNU__)
  #define DEFAULT_PRESET 3
#elif defined(_WIN32) || defined(_WIN64) || defined(__MSYS__)
  #define DEFAULT_PRESET 4
#elif defined(__unix__)
  #define DEFAULT_PRESET 5
#endif

#define MS_DISS_PRESET_NAME "m$"

preset_t presets[] = {
  {"mac",     "macOS"},
  {"bsd",     "*BSD"},
  {"linux",   "Linux"},
  {"hurd",    "GNU/Hurd"},
  {"windows", "Windows"},
  {"unix",    "*nix"},
  {"deck",    "Steam Deck"},
  {"reactos", "ReactOS"},
  {MS_DISS_PRESET_NAME, "diss M!cr0$0f+"}
};

int lang_id = -1;
int preset_id = DEFAULT_PRESET;

void i18n_set_lang_id(void) {
  if (lang_id != -1)
    return;

#ifdef GDI
  // LCIDToLocaleName is available starting from Vista
  // if you want to compile activate-linux for XP and ReactOS
  // please, do one of:
  // * compile with https://github.com/Chuyu-Team/VC-LTL/blob/master/src/ucrt/locale/lcidtoname_downlevel.cpp
  // * replace code below with: char lang[] = "ru_RU";
  #define LANG_STR_SIZE 6
  wchar_t langw[LANG_STR_SIZE];
  LCIDToLocaleName(LOCALE_USER_DEFAULT, langw, LANG_STR_SIZE, 0);
  char lang[LANG_STR_SIZE];
  for (int i=0; i<LANG_STR_SIZE; i++) lang[i] = langw[i];
  lang[2] = '_';
  #undef LANG_STR_SIZE
#else
  char *lang = getenv("LANG");
#endif

  __info__("Got user language %s\n", lang);
  for (lang_id = length(langs); lang_id; lang_id--)
    if (match_str(langs[lang_id].code, lang))
      return;
  if (!match_str(langs[lang_id].code, lang)) {
    __error__("activate-linux lacks translation for `%s' language. You are welcome to fix this :3\n", lang);
    __error__("Using English translation\n");
  }
}

void i18n_set_preset(const char *const preset) {
  if (!preset)
    return;

  if (match_str(MS_DISS_PRESET_NAME, preset)) {
    if (!(langs[lang_id].diss.subtitle)) {
      __error__("Diss for `%s' is currently not translated. You are welcome to fix this :3\n", langs[lang_id].code);
      __error__("Using English diss\n");
      lang_id = 0;
    }
    return;
  } else {
    for (preset_id = length(presets)-1; preset_id >= 0; preset_id--) {
      if (match_str(presets[preset_id].name, preset))
        return;
    }
  }

  __error__("Undefined preset: %s\n", preset);
  i18n_list_presets();
  exit(EXIT_FAILURE);
}

void *allocated[] = {NULL, NULL};
void i18n_set_info(const char *const preset) {
  i18n_set_lang_id();

  __info__("Loading preset: %s\n", preset);
  i18n_set_preset(preset);
  __info__("Loaded preset: %s\n", presets[preset_id].name);

  if (!allocated[0]) options.title    = allocated[0] = malloc(666);
  if (!allocated[1]) options.subtitle = allocated[1] = malloc(666);

  memset(options.title, 0, 666);
  memset(options.subtitle, 0, 666);

  if (match_str(MS_DISS_PRESET_NAME, preset)) {
    strcat(options.title, langs[lang_id].diss.pre_title);
    strcat(options.title, presets[preset_id].text);
    strcat(options.title, langs[lang_id].diss.post_title);

    strcat(options.subtitle, langs[lang_id].diss.subtitle);
  } else {
    strcat(options.title, langs[lang_id].windows_like.pre_title);
    strcat(options.title, presets[preset_id].text);
    strcat(options.title, langs[lang_id].windows_like.post_title);

    strcat(options.subtitle, langs[lang_id].windows_like.pre_subtitle);
    strcat(options.subtitle, presets[preset_id].text);
    strcat(options.subtitle, langs[lang_id].windows_like.post_subtitle);
  }
}

void i18n_list_presets(void) {
  #define HELP(fmtstr, ...) fprintf(stderr, fmtstr "\n", ## __VA_ARGS__)
  #define STYLE(x) "\033[" # x "m"
  fprintf(stderr, "Built-in Presets:\n\n");

  HELP(STYLE(1) "Name\t\tDescription" STYLE(0));
  for (size_t len = 0; len < length(presets); len++)
    HELP(STYLE(1) "%s" STYLE(0) "\t\tPlatform preset for %s", presets[len].name, presets[len].text);
  #undef STYLE
  #undef HELP
}
