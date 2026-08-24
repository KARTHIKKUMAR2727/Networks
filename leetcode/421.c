int findMaximumXOR(int* nums, int numsSize) {
    int ans = 0;
    int mask = 0;

    for (int i = 30; i >= 0; i--) {
        mask |= (1 << i);

        int prefix[100000];
        int size = 0;

        for (int j = 0; j < numsSize; j++)
            prefix[size++] = nums[j] & mask;

        int candidate = ans | (1 << i);

        for (int j = 0; j < size; j++) {
            for (int k = j + 1; k < size; k++) {
                if ((prefix[j] ^ prefix[k]) == candidate) {
                    ans = candidate;
                    goto found;
                }
            }
        }

        found:;
    }

    return ans;
}