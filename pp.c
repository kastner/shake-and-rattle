/*
 * I dunno, pairing or something
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "psmove_private.h"
#include "psmove.c"
#include "hidapi.h"


int main(void)
{
    PSMove *moves[7];

    int i;
    int move_count = psmove_count_connected();
    printf("Connected controllers: %d\n", move_count);

    if (move_count < 1) {
        printf("You must have at least 1 controller!\n");
        exit(1);
    }

    for (i = move_count-1; i >= 0; i--) {
        moves[i] = psmove_connect_by_id(i);
        if (moves[i] == NULL) {
            printf("Could not connect to controller\n");
            exit(1);
        }
        else {
            PSMove *move = moves[i];
            // do something here
            unsigned char *whatever;
            int len;
            while (1) {
                psmove_set_leds(move, 0, 0, 200);
                psmove_set_rumble(move, 0);
                //hid_set_nonblocking(move->handle, 0);
                //if ((len = hid_read_timeout(move->handle, whatever, 10, 10))) {
                    //printf("data? %d", len);
                    //_psmove_read_data(moves[i], whatever, len);
                //    printf("data: %s\n", whatever);
               // }
                //else {
                //    printf("waiting...\n");
               // }
                //psmove_update_leds(move);
    unsigned char btg[128];
    int j;
  
    for (j = 0x11; j <= 255; j++) {
       memset(btg, 0, sizeof(btg));
        // 0x11
        btg[0] = j;
        int res = hid_get_feature_report(move->handle, btg, 128);

    if (res > 0) {
    printf("res for %#x: %d : ", j, res);
      for (i = 0; i < res; i++) {
        printf("%#x ", btg[i]);
      }
    printf("\n");
    }
}
    printf("\n\n");

/*
    if (res == sizeof(btg)) {
        if (controller != NULL) {
            memcpy(*controller, btg+1, 6);
        }
*/
                sleep(1);
            }
            //printf("serial: %s\n", psmove_get_serial(moves[i]));
        }
    }

    for (i = move_count-1; i >= 0; i--) {
        psmove_disconnect(moves[i]);
    }

    return 0;
}

