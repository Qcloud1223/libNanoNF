//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

pcre2_code *re[4];
pcre2_match_data *matchData[4];
const PCRE2_SPTR patterns[] = {
    "car\\S{1,32}rot",
    "", //"\\S{1,10}@\\S{1,10}\\.\\S{1,3}",
    "forsigcomm",
    "correctizer"};

void InitializeFunction()
{
    int errorNumber;
    PCRE2_SIZE errorOffset;
    //pcre2_code *re[4];
    //PCRE2_SPTR patterns[] = {"apple", "", "pen", "pear"};
    for (int i = 0; i < 4; i += 1)
    {
        printf("[match] compile re\n");
        re[i] = pcre2_compile(
            patterns[i], PCRE2_ZERO_TERMINATED, 0, &errorNumber, &errorOffset, NULL);
        printf("[match] compile done\n");
        if (!re[i])
        {
            PCRE2_UCHAR buffer[256];
            pcre2_get_error_message(errorNumber, buffer, sizeof(buffer));
            fprintf(stderr, "PCRE2 compilation failed at offset %d: %s\n", (int)errorOffset,
                    buffer);
            exit(1);
        }
        matchData[i] = pcre2_match_data_create_from_pattern(re[i], NULL);
        //printf("actively doing nothing except print initialize here\n");
    }
}

void ProcessPacket(uint8_t *packet, size_t packetLength)
{
    for (int i = 0; i < 4; i += 1)
    {
        pcre2_match(re[i], packet, packetLength, 0, 0, matchData[i], NULL);
        //printf("actively doing nothing except print process here\n");
    }
}