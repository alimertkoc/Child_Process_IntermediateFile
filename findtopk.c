
/*
    Muhammet Onur Bayraktar 170419821
    Bahadır Şahin           170419822
    Ali Mert Koç            170418018
*/

/*
    fork() ve ara dosyalar kullanarak
    N adet dosya icerisindeki k. en buyuk
    sayiyi bulan ve buldugu degerleri
    bir cikti dosyasina yazan programdir.     
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>


/*
    k. en buyuk elemani bulan fonksiyondur.
    Bubble sort algoritmasi k + 1 kez calistirilinca
    k. en buyuk eleman dizinin sondan k. elemani olur.
*/
int findTopK(int *arr, int k, int n)
{
    int tmp = 0;

    for (int i = 0; i < k + 1; i++) // dongu k + 1 kez calistirilacak
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            // su anki eleman sonrakinden buyukse yer degistirir
            if (arr[j] > arr[j + 1])
            {
                tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }

    return arr[n - k - 1]; // Sondan k. elemani dondurur
}

/*
    Her bir child processten gelen int degerlerinin
    dosyaya yazilmadan once buyukten kucuge siralanmasi
    icin yazildi.
*/
void sort(int *arr, int n)
{
    int tmp;

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (arr[j] < arr[j + 1])
            {
                tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

/*
    Ciktilarin dogru olup olmadigini gorebilmek
    icin yazildi.
*/
void printArray(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        printf("%d, ", arr[i]);
    }

    // En sona virgul koymamak icin
    printf("%d\n", arr[n - 1]);
}

/*
    Girdi dosyalarinin icindeki
    integer degerleri okuyup,
    diziye ekledikten sonra
    k. elemani ara dosyaya yazar.
*/

void generateIntFiles(int k, int N, char filenames[][20])
{
    int fd, tmp_num_index = 0, input_index = 0, f_int;
    char tmp_num[11], tmp[2], intermediate[2], result[12];
    
    // dosyada kac adet int deger oldugu bilinmedigi icin
    static int inputs[INT_MAX];

    // N adet child process olusturup
    // gerekli islemleri yapan kisim

    for (int i = 0; i < N; i++)
    {
        pid_t c = fork();

        if (c == 0) // donguye giren child process ise
        {
            // parametre olarak gelen dosya adini kullanarak
            // ilgili dosyayi okuma modunda acar
            fd = open(filenames[i], O_RDONLY);

            // Herhangi bir karakter okunduysa donguye girer
            // Hicbir karakterin okunmamasi dosyanin sonuna
            // gelindigini gosterir.
            while(read(fd, tmp, 1) != 0)
            {
                // Okunan karakterin bosluk olmasi bir sayinin
                // tamamen okundugunu gosterir.
                if (tmp[0] == ' ') // Okunan karakter bosluksa
                {
                    // Okunan karakterlerden olusan diziyi 
                    // string olarak okuyabilmek icin dizinin
                    // sonuna \0 karakteri eklendi.
                    tmp_num[tmp_num_index] = '\0';
                    tmp_num_index = 0;

                    // String deger olarak tutulan sayiyi integer
                    // tipinee donusturur
                    sscanf(tmp_num, "%d", &inputs[input_index]);
                    input_index++;
                }

                else
                {
                    // Okunan karakter bosluk degilse sayi tamamlanmamistir
                    // Bu durumda okunan karakter diziye eklenmelidir
                    tmp_num[tmp_num_index] = tmp[0];
                    tmp_num_index++;
                }
            }

            close(fd); // Dosya is bittikten sonra kapatildi.

            /* Ara dosyanin olusturulmasi */

            // Ara dosya ismi olarak o anki i degeri secildi
            // ve stringe cevrildi.
            sprintf(intermediate, "%d", i);
            
            // Dosya yazma modunda acildi.
            // Burada S_IRUSR girilmezse dosyaya sadece
            // superuser erisebilir. 
            f_int = open(intermediate, O_CREAT | O_WRONLY, S_IRUSR);

            // k. en buyuk sayiyi bulur ve dosyaya yazabilmek icin
            // stringe donusturur.
            sprintf(result, "%d", findTopK(inputs, k, input_index));
            write(f_int, result, strlen(result));
            close(f_int);
            exit(0);

            /* ONEMLI */
            /* 
                Child processler buraya giremez.
            */
        }
    }

    // Child processler buraya giremeyecegi icin
    // parent processin N kez wait fonksiyonunu cagirmasi 
    // gerekir. Aksi takdirde parent, child processler tamamlanmadan
    // islemlerine devam eder.
    for(int i = 0; i < N; i++)
        wait(NULL);
}

void main(int argc, char *argv[])
{
    /*
        Program execute edilirken verilen
        argumanlar degiskenlere atanir.
    */
    int k, N, f_int, numbers[N], number, tmp_num_index = 0;

    // Alinan k ve N degerleri int tipine cevrilir.
    sscanf(argv[1], "%d", &k);
    sscanf(argv[2], "%d", &N);

    char filenames[N][20], tmp[2], outfile[20], tmp_num[12], result[12], tmp_filename[2];

    // Dosya adlari teker teker filenames dizisine atanir.
    for (int i = 3; i < argc - 1; i++)
    {
        strcpy(filenames[i - 3], argv[i]);
    }

    strcpy(outfile, argv[argc - 1]);

    generateIntFiles(k, N, filenames);

    // Burasi child processler terminate edildikten sonra calisir.
    for (int j = 0; j < N; j++)
    {
        sprintf(tmp_filename, "%d", j); // Ara dosyalarin adlari olusturulur
        f_int = open(tmp_filename, O_RDONLY); // Ara dosyalar okuma modunda acilir.
        tmp_num_index = 0;

        while (read(f_int, tmp, 1) != 0) // Dosyanin sonuna kadar okumak icin
        {
            tmp_num[tmp_num_index] = tmp[0];
            tmp_num_index++;
        }

        tmp_num[tmp_num_index] = '\0';
        sscanf(tmp_num, "%d", &number); // Dosyadan okunan deger int tipine cevrilir.
        numbers[j] = number;
        remove(tmp_filename); // ara dosyalar silinir.
    }
    
    sort(numbers, N);
    printf("****Intermediate Files****\n"); // make komutunun calismasini test etmek icin
    // printArray(numbers, N);

    // Ara dosyalardan okunan degerler siralandiktan sonra dosyaya yazilacak.
    int out_fd = open(outfile, O_CREAT | O_WRONLY, S_IRUSR);

    for (int k = 0; k < N; k++)
    {
        sprintf(result, "%d", numbers[k]);
        strcat(result, "\n");
        write(out_fd, result, strlen(result));
    }

    close(out_fd);
}