/*
 * Some game thing
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "psmove.h"

int main(int argc, char* argv[])
{
    //PSMove *move, *move2;
    PSMove *moves[7];

    int last_xs[7];
    int last_ys[7];
    int last_zs[7];
    int ready[7];

    //int state = 0;

    float decay = 0.009;
    float increase = 0.004;
    
    // amount to get to win
    float win = 0.99;

    int told_everyone = 0;

    int dist_thresh = 1000;

    float vals[7];

    struct rgb { int r, g, b; };

    int colors[7][3];


    /* Red */
    colors[0][0] = 255;
    colors[0][1] = 0;
    colors[0][2] = 0;

    /* Orange */
    colors[1][0] = 255;
    colors[1][1] = 127;
    colors[1][2] = 0;

    /* Yellow */
    colors[2][0] = 255;
    colors[2][1] = 255;
    colors[2][2] = 0;

    /* Green */
    colors[3][0] = 0;
    colors[3][1] = 255;
    colors[3][2] = 0;

    /* Blue */
    colors[4][0] = 0;
    colors[4][1] = 0;
    colors[4][2] = 255;

    /* Indigo */
    colors[5][0] = 0;
    colors[5][1] = 0;
    colors[5][2] = 128;

    /* Voilet */
    colors[6][0] = 143;
    colors[6][1] = 0;
    colors[6][2] = 255;

    int i, ii, j, move_count;

    for (i = 7; i >= 0; i--) { 
      vals[i] = 0;
      last_xs[i] = 0;
      last_ys[i] = 0;
      last_zs[i] = 0;
      ready[i] = -1;
    }

    move_count = psmove_count_connected();
    printf("Connected controllers: %d\n", move_count);

    for (i = move_count-1; i >= 0; i--) {
        moves[i] = psmove_connect_by_id(i);
        if (moves[i] == NULL) {
            printf("Could not connect to controller\n");
            exit(1);
        }
    }

    /* Enable rate limiting for LED updates */
    //psmove_set_rate_limiting(move, 1);

    // reset
    for (i = move_count-1; i >= 0; i--) {
        psmove_set_leds(moves[i], 0, 0, 0);
        psmove_set_rumble(moves[i], 0);
        psmove_update_leds(moves[i]);
    }

    int color = -1;
    while (!(psmove_get_buttons(moves[0]) & Btn_PS)) {
        for (i = move_count-1; i >= 0; i--) {
            PSMove *move = moves[i];
            int res = psmove_poll(move);
            if (res) {
                color++;
                color = color % 7;

                if (ready[i] != -1) {
                  //printf("trigger: %d i: %i\n", psmove_get_trigger(move), i);
                  if (psmove_get_trigger(move) >= 100) {
                    //ready[i] = 0;
                    for (j=0; j<10; j++) {
                        psmove_set_leds(moves[i], colors[color][0] * win, colors[color][1] * win, colors[color][2] * win);
                        //psmove_set_rumble(move, 255*(j%2));
                        psmove_update_leds(move);
                        usleep(10000*(j%10));
                    }
                    psmove_set_rumble(move, 0);
                    psmove_update_leds(move);
                  }
                  break;
                }

                int ready_to_go = 1;

/*
                for (ii = move_count-1; ii >= 0; ii--) {
                    if (ready[ii] < 0) { ready_to_go = 0; break; }
                }
*/

                if (! ready_to_go) { break; }

/*
                if (! told_everyone) {
                    told_everyone = 1;
                    for (ii = move_count-1; ii >= 0; ii--) {
                        for (j=0; j<10; j++) {
                            psmove_set_leds(moves[ii], colors[ii][0] * win, colors[ii][1] * win, colors[ii][2] * win);
                            psmove_set_rumble(moves[ii], 255*(j%2));
                            psmove_update_leds(move);
                            usleep(10000*(j%10));
                        }
                        psmove_update_leds(moves[ii]);
                        printf("GAME START!!!!!!!\n");
                    }

                    sleep(1);

                    for (ii = move_count-1; ii >= 0; ii--) {
                        psmove_set_leds(moves[ii], 0, 0, 0);
                        psmove_set_rumble(moves[ii], 0);
                        psmove_update_leds(moves[ii]);
                    }
                }
*/
                int x, y, z;
                psmove_get_gyroscope(move, &x, &y, &z);
                int diff_x = x - last_xs[i];
                int diff_y = y - last_ys[i];
                int diff_z = z - last_zs[i];

                float distance = sqrt((diff_x * diff_x) + (diff_y * diff_y) + (diff_z * diff_z));

                if (distance > dist_thresh) {
                    psmove_set_leds(moves[i], colors[color][0] * win, colors[color][1] * win, colors[color][2] * win);
                    //vals[i] = vals[i] + increase;
                }

                if (distance < dist_thresh) {
                    vals[i] = vals[i] - decay;
                    if (vals[i] <= 0) { vals[i] = 0; }
                }

                printf("val%d is now %f\n", i, vals[i]);

                //psmove_set_leds(move, colors[i][0] * vals[i], colors[i][1] * vals[i], colors[i][2] * vals[i]);
                psmove_update_leds(move);

/*
                if (vals[i] >= win) {
                    // game over! buzz the winner and let everyone else know!
                    for (j=0; j<2; j++) {
                        psmove_set_rumble(move, 255*(j%2));
                        psmove_update_leds(move);
                        usleep(10000*(j%10));
                        printf("won!... buzz\n");
                    }

                    for (ii = move_count-1; ii >= 0; ii--) {
                        psmove_set_leds(moves[ii], colors[i][0] * vals[i], colors[i][1] * vals[i], colors[i][2] * vals[i]);
                        psmove_update_leds(moves[ii]);
                        printf("win - COLORS!\n");
                    }

                    sleep(3);

                    for (ii = move_count-1; ii >= 0; ii--) {
                        psmove_set_leds(moves[ii], 0, 0, 0);
                        psmove_set_rumble(moves[ii], 0);
                        psmove_update_leds(moves[ii]);
                        vals[i] = 0.0;
                        ready[ii] = -1;
                        told_everyone = 0;
                        printf("won!... reset\n");
                    }
                }
*/
            }
            usleep(1000);
        }
    }

    for (i = move_count-1; i >= 0; i--) {
        psmove_disconnect(moves[i]);
    }

    return 0;
}

