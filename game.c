/*
 * Some game thing
 *
 * Erik Kastner <kastner@gmail.com>
 * Hacked mostly during XOXO. Inspired by JSJoust.
 *
 * 2-7 players. Pair the controllers then launch the game.
 * Each player hits the trigger to register
 * once everyone is registered, everyone's colors will be shown
 * then the balls will go white and everyone shakes as fast as they can
 * as you shake, your color fills up, the first one to fill it all the way wins
 *
 * just hit the trigger after a round to start again
 *
 *
 * allow overriding of game variables with environment vars
 *
 * DIST_THRESH: (int) amount needed to move
 * WIN_VAL: (float, 0-1, default: 0.99) total score to get to for a win (also controls max color)
 * DECAY: (float, 0-1, default: 0.009) how much score decreases each time it's not moved enough
 * INCREASE: (float, 0-1, default: 0.04) how much score increase each time it is moved enough
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <psmoveapi/psmove.h>

int main(void)
{
    PSMove *moves[7];

    int last_xs[7];
    int last_ys[7];
    int last_zs[7];
    int ready[7];

    // how much to decay when the controller hasn't moved enough
    float decay = 0.009;

    // how much to increase when it has moved enough
    float increase = 0.004;
    
    // amount to get to win
    float win = 0.99;

    int told_everyone = 0;

    float vals[7];

    int colors[7][3];

    colors[0][0] = 0;
    colors[0][1] = 255;
    colors[0][2] = 0;

    colors[1][0] = 255;
    colors[1][1] = 0;
    colors[1][2] = 0;

    colors[2][0] = 0;
    colors[2][1] = 0;
    colors[2][2] = 255;

    colors[3][0] = 128;
    colors[3][1] = 128;
    colors[3][2] = 0;

    colors[4][0] = 0;
    colors[4][1] = 128;
    colors[4][2] = 128;

    colors[5][0] = 128;
    colors[5][1] = 0;
    colors[5][2] = 128;

    colors[6][0] = 64;
    colors[6][1] = 64;
    colors[6][2] = 64;

    int i, ii, j, move_count;

    // amount needed to move to get an increase
    int dist_thresh = 1000;


    char *env_dist_thresh = getenv("DIST_THRESH");
    if (env_dist_thresh != NULL) { dist_thresh = atoi(env_dist_thresh); }

    char *env_win = getenv("WIN_VAL");
    if (env_win != NULL) { win = atof(env_win); }

    char *env_decay = getenv("DECAY");
    if (env_decay != NULL) { decay = atof(env_decay); }

    char *env_increase = getenv("INCREASE");
    if (env_increase != NULL) { decay = atof(env_increase); }


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

    while (!(psmove_get_buttons(moves[0]) & Btn_PS)) {
        for (i = move_count-1; i >= 0; i--) {
            PSMove *move = moves[i];
            int res = psmove_poll(move);
            if (res) {
                if (ready[i] == -1) {
                  if (psmove_get_trigger(move) >= 100) {
                    ready[i] = 0;
                    for (j=0; j<10; j++) {
                        psmove_set_rumble(move, 255*(j%2));
                        psmove_update_leds(move);
                        usleep(10000*(j%10));
                    }
                    psmove_set_rumble(move, 0);
                    psmove_update_leds(move);
                  }
                  break;
                }

                int ready_to_go = 1;

                for (ii = move_count-1; ii >= 0; ii--) {
                    if (ready[ii] < 0) { ready_to_go = 0; break; }
                }

                if (! ready_to_go) { break; }

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

                int x, y, z;
                psmove_get_gyroscope(move, &x, &y, &z);
                int diff_x = x - last_xs[i];
                int diff_y = y - last_ys[i];
                int diff_z = z - last_zs[i];

                // update last pos
                last_xs[i] = x;
                last_ys[i] = y;
                last_zs[i] = z;

                float distance = sqrt((diff_x * diff_x) + (diff_y * diff_y) + (diff_z * diff_z));

                if (distance > (float)dist_thresh) {
                    vals[i] = vals[i] + increase;
                }

                if (distance < (float)dist_thresh) {
                    vals[i] = vals[i] - decay;
                    if (vals[i] <= 0) { vals[i] = 0; }
                }

                printf("val%d is now %f\n", i, vals[i]);

                psmove_set_leds(move, colors[i][0] * vals[i], colors[i][1] * vals[i], colors[i][2] * vals[i]);
                psmove_update_leds(move);

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
            }
            usleep(1000);
        }
    }

    for (i = move_count-1; i >= 0; i--) {
        psmove_disconnect(moves[i]);
    }

    return 0;
}

