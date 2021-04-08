#include "afproto.h"
#include "fcs16.h"

int afproto_get_data(const char *src, unsigned int src_len, char *dest,
                     unsigned int *dest_len) {
  unsigned int i;
  static unsigned char control_escape, value;
  static unsigned short fcs = FCS16_INIT_VALUE;
  static int ret, start_index = -1, end_index = -1, src_index = 0, dest_index =
      0;

  // Run through the data
  for (i = 0; i < src_len; i++) {
    // First find the start flag sequence
    if (start_index < 0) {
      if (src[i] == AFPROTO_FLAG_SEQUENCE) {
        // Check if an additional flag sequence byte is present
        if ((i < (src_len - 1)) && (src[i + 1] == AFPROTO_FLAG_SEQUENCE)) {
          // Just loop again to silently discard it (accordingly to RFC 1662)
          continue;
        }

        start_index = src_index;
      }
    } else {
      // Check for end flag sequence
      if (src[i] == AFPROTO_FLAG_SEQUENCE) {
        // Check if an additional flag sequence byte is present or was earlier received
        if (((i < (src_len - 1)) && (src[i + 1] == AFPROTO_FLAG_SEQUENCE))
            || ((start_index + 1) == src_index)) {
          // Just loop again to silently discard it (accordingly to RFC 1662)
          continue;
        }

        end_index = src_index;
        break;
      } else if (src[i] == AFPROTO_CONTROL_ESCAPE) {
        control_escape = 1;
      } else {
        // Update the value based on any control escape received
        if (control_escape) {
          control_escape = 0;
          value = src[i] ^ 0x20;
        } else {
          value = src[i];
        }

        // Now update the FCS and add it to the destination buffer
        fcs = fcs16(fcs, value);
        dest[dest_index++] = value;
      }
    }
    src_index++;
  }

  // Check for invalid frame (no start or end flag sequence) and FCS error (less than 2 bytes frame)
  if ((start_index < 0) || (end_index < 0)) {
    // Return no start or end flag sequence and make sure destination length is 0
    *dest_len = 0;
    ret = -1;
  } else if ((end_index < (start_index + 2)) || (fcs != FCS16_GOOD_VALUE)) {
    // Return FCS error and indicate that data up to end flag sequence in buffer should be discarded
    *dest_len = i;
    ret = -2;
  } else {
    // Return success and indicate that data up to end flag sequence in buffer should be discarded
    *dest_len = dest_index - 2;
    ret = i;
  }

  // Reset values for next frame if start and end flag sequence has been detected
  if (ret != -1) {
    fcs = FCS16_INIT_VALUE;
    src_index = dest_index = 0;
    start_index = end_index = -1;
  }

  return ret;
}

void afproto_frame_data(const char *src, unsigned int src_len, char *dest,
                        unsigned int *dest_len) {
  unsigned char value;
  unsigned int i, dest_index = 0;
  unsigned short fcs = FCS16_INIT_VALUE;

  // Start by adding the start flag sequence
  dest[dest_index++] = AFPROTO_FLAG_SEQUENCE;

  // Run through the data
  for (i = 0; i < src_len; i++) {
    // Calculate FCS value before escaping any characters
    fcs = fcs16(fcs, src[i]);

    // Check for characters to be escaped
    if ((src[i] == AFPROTO_FLAG_SEQUENCE)
        || (src[i] == AFPROTO_CONTROL_ESCAPE)) {
      dest[dest_index++] = AFPROTO_CONTROL_ESCAPE;
      value = src[i] ^ 0x20;
    } else {
      value = src[i];
    }

    dest[dest_index++] = value;
  }

  // Invert the FCS value accordingly to RFC 1662
  fcs ^= 0xFFFF;

  // Run through the FCS bytes
  for (i = 0; i < sizeof(fcs); i++) {
    value = ((fcs >> (8 * i)) & 0xFF);

    // Check for characters to be escaped
    if ((value == AFPROTO_FLAG_SEQUENCE) || (value == AFPROTO_CONTROL_ESCAPE)) {
      dest[dest_index++] = AFPROTO_CONTROL_ESCAPE;
      value ^= 0x20;
    }

    dest[dest_index++] = value;
  }

  // Add end flag sequence and update length of frame
  dest[dest_index++] = AFPROTO_FLAG_SEQUENCE;
  *dest_len = dest_index;
}
