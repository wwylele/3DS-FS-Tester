#include "util.h"

std::string GetCommand() {
    static SwkbdState swkbd;
	static char mybuf[60];

	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, -1);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetHintText(&swkbd, "Please input a command.");

	if (SWKBD_BUTTON_RIGHT != swkbdInputText(&swkbd, mybuf, sizeof(mybuf)))
        return "";

    return mybuf;
}
