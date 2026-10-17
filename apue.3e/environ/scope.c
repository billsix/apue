int f1(int val) {
    int num = 0;
    const int *ptr = &num;

    if (val == 0) {
        int val;

        val = 5;
        ptr = &val;
    }
    return (*ptr + 1);
}
