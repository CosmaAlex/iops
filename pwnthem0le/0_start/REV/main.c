#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_LEN 32

const char dec_key[KEY_LEN] = {0xbb, 0xcb, 0x83, 0xfe,
				0x80, 0x93, 0x51, 0xcb,
				0x95, 0x9b, 0xbf, 0xb3,
				0xa1, 0xc2, 0x14, 0x08,
				0x33, 0x3f, 0x8c, 0xbc,
				0x6a, 0xca, 0x43, 0x4b,
				0xa1, 0x4f, 0x8f, 0xaa,
				0x8d, 0x00, 0x00, 0x00 };

int main(void) {
	int i, seed=0;
	char tmp_key[KEY_LEN];
	
	memcpy(tmp_key, dec_key, KEY_LEN*sizeof(char));
	while (tmp_key[0] != 'p' || tmp_key[1] != 't' || tmp_key[2] != 'm' || tmp_key[3] != '{') {
		memcpy(tmp_key, dec_key, KEY_LEN*sizeof(char));

		srand(seed);
		for (i=0; i < (int) strlen(tmp_key); i++) {
			int rand_val = rand();
			tmp_key[i] = tmp_key[i] ^ (char) rand_val + (char)(rand_val/0xff);
		}

		seed++;
	}

	printf("The seed required was %d\nThe decrypted flag is %s\n\nn i c e\n\n", seed-1, tmp_key);
	return 0;
}
