#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// count the number of digits 
int nrDigits(int num) {
    int count = 0;
    if(num == 0){
        count++;
    }
    while (num != 0) {
        
        num /= 10;
        count++;
    }
    return count;
}

// check if a number is prime
int isPrime(int num) {
    if (num <= 1)
        return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0)
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

    // create pipes
    int p1_to_p2[2], p1_to_p3[2], p2_to_p1[2], p3_to_p1[2];
    if (pipe(p1_to_p2) == -1 || pipe(p1_to_p3) == -1 || pipe(p2_to_p1) == -1 || pipe(p3_to_p1) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid_t pid_p2, pid_p3;

    // create process P2
    pid_p2 = fork();
    if (pid_p2 < 0) {
        perror("Fork failed");
        return 1;
    }
    if (pid_p2 == 0) { // child process P2
        close(p1_to_p2[1]); // close unused write end
        close(p1_to_p3[0]); // close unused read end
        close(p2_to_p1[0]); // close unused read end
        close(p3_to_p1[0]); // close unused read end

        int count_digits[6] = {0}; // counts for each digit
        int num;
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            perror("File opening failed");
            exit(1);
        }

        while (fscanf(file, "%d", &num) != EOF) {
            int digits = nrDigits(num);
            count_digits[digits]++;
        }
        fclose(file);

        // send results to parent
        write(p2_to_p1[1], count_digits, sizeof(count_digits));
        close(p1_to_p2[0]);
        close(p2_to_p1[1]);
        exit(0);
    }

    // create process P3
    pid_p3 = fork();
    if (pid_p3 < 0) {
        perror("Fork failed");
        return 1;
    }
    if (pid_p3 == 0) { // child process P3
        close(p1_to_p2[0]); // close unused read end
        close(p1_to_p3[1]); // close unused write end
        close(p2_to_p1[0]); // close unused read end
        close(p3_to_p1[0]); // close unused read end

        int primes = 0;
        int num;
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            perror("File opening failed");
            exit(1);
        }

        while (fscanf(file, "%d", &num) != EOF) {
            if (isPrime(num))
                primes++;
        }
        fclose(file);

        // send results to parent
        write(p3_to_p1[1], &primes, sizeof(int));
        close(p1_to_p3[0]);
        close(p3_to_p1[1]);
        exit(0);
    }

    // parent process P1
    close(p1_to_p2[0]); // close unused read end
    close(p1_to_p3[0]); // close unused read end
    close(p2_to_p1[1]); // close unused write end
    close(p3_to_p1[1]); // close unused write end

    // wait for child processes to terminate
    wait(NULL);
    wait(NULL);

    // results from child processes
    int count_digits[6];
    int primes;

    read(p2_to_p1[0], count_digits, sizeof(count_digits));
    read(p3_to_p1[0], &primes, sizeof(int));

    int total_numbers = 0;
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("File opening failed");
        return 1;
    }
    int num;
    while (fscanf(file, "%d", &num) != EOF) {
        total_numbers++;
    }
    fclose(file);

    // print results
    printf("Input file: %s\n", filename);
    printf("\n");
    printf("1 digits - %d\n", count_digits[1]);
    printf("2 digits - %d\n", count_digits[2]);
    printf("3 digits - %d\n", count_digits[3]);
    printf("4 digits - %d\n", count_digits[4]);
    printf("5 digits - %d\n", count_digits[5]);
    printf("Primes - %d\n", primes);
    printf("Nonprimes - %d\n", total_numbers - primes);

    return 0;
}
