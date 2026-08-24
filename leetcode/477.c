int totalHammingDistance(int* nums, int numsSize) {
    int result = 0;

    for (int bit = 0; bit < 31; bit++) {
        int ones = 0;

        for (int i = 0; i < numsSize; i++)
            ones += (nums[i] >> bit) & 1;

        result += ones * (numsSize - ones);
    }

    return result;
}