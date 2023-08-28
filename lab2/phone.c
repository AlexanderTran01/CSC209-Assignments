#include <stdio.h>
int main(int argc, char **argv) {
    char phone[11];
    int index;
    scanf("%s%d", phone, &index);
    if (index == -1) {
        printf("%s", phone);
        return 0;
    } else if (index < -1 || index > 9){
        printf("ERROR");
        return 1;
    } else {
        printf("%c", phone[index]);
        return 0;
    }
}