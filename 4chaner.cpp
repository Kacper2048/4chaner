#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <curl/curl.h>
#include <fcntl.h>
#include <unistd.h>


using namespace std;

struct excel
{
    void * ptr_buffer = nullptr;
    void * ptr = nullptr;

    size_t nowyr_pamieci = 0;
    size_t r_pamieci = 0;
};

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *baza_danych);
vector<string> szukanie_frazy(excel * baza_danych);
vector<string> szukanie_nazwy(vector<string>);
excel pobranie_pliku(vector<string> link);
void zapis_plikow(excel wsk_do_plikow,vector<string> nazwy_plikow,string sciezka);
string sciezka(); //tworzy katalog dla nowych likow

int main()
{
    string link_do_strony;

    curl_global_init(CURL_GLOBAL_ALL);
    vector<string> linki;
    excel excel_pliki;
    vector<string> nazwy_plikow;
    CURL * curl =  curl_easy_init();
    CURLcode res;
    excel Baza_danych;

    if(curl)
    {
        string path = sciezka(); //tworzy katalog na pliki

        //ustawia polaczenie ze stroną
        do
        {
            cout << "podaj linka do danego wątku: "; cin >> link_do_strony;
            cout << "trwa łączenie..." << endl;
            curl_easy_setopt(curl, CURLOPT_URL, link_do_strony.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&Baza_danych);

            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                cout << curl_easy_strerror(res) << endl;
            }

        }while(res != CURLE_OK);

        cout << "polaczenie udane <3" << endl;

        //pozyskanie linkow
        linki = szukanie_frazy(&Baza_danych);
        cout << "szukanie linkow udane" << endl;
        nazwy_plikow = szukanie_nazwy(linki);
        cout << "szukanie nazw plikow udane" << endl;

        //pokazanie linkow
        for(int i = 0; i< linki.size();i++)
        {
            cout << linki[i] << "   ---   " << nazwy_plikow[i] << endl;
        }

        for(int i=0;i<linki.size();i++)
        {
            excel_pliki = pobranie_pliku(linki);
            zapis_plikow(excel_pliki,nazwy_plikow,path);
            cout << "plik nr:" << i+1 << " z " << linki.size() << "   "<< static_cast<float>(excel_pliki.r_pamieci/1024) << " KB" <<endl;
        }

    }
    else
    {
        fprintf(stderr,"curl odmówił posłuszeństwa");
    }
    
    //czyszczenie
    free(Baza_danych.ptr_buffer);
    curl_easy_cleanup(curl);
    return 0;
}

size_t write_callback(char *zrodlo, size_t size, size_t nmemb, void *baza_danych)
{
    struct excel * skrot = (struct excel *)baza_danych;
    //int static ile = 0;
    
    //tworzenie nowej przestrzeni z nowym rozmiarem
    skrot->nowyr_pamieci = skrot->r_pamieci + (size * nmemb);
    skrot->ptr = malloc(skrot->nowyr_pamieci);
    if(skrot->ptr == NULL)
    {
        cout << "coś nie pykło z alokacją pamieci :/" << endl;
    }
    else
    {
        //kopiowanie starej zawartosci
        memcpy(skrot->ptr,skrot->ptr_buffer,skrot->r_pamieci);

        //kopiowanie nowej zawartosci
        memcpy( (skrot->ptr + skrot->r_pamieci),(void * )zrodlo, size * nmemb);

        //usuwanie starej przestrzeni
        free(skrot->ptr_buffer);
    
        //zapis poprzedniej sesji
        skrot->ptr_buffer = skrot->ptr;
        skrot->r_pamieci = skrot->nowyr_pamieci;

        return size * nmemb;
    }
    return 0;
}

vector<string> szukanie_frazy(excel * baza_danych)
{
    string napis_wyciety;
    //dopóki nie rozgryze o chuj chodzi
    string link_zakazany = "i.4cdn.org";

    vector<string> linki;
    string link;
    string const szukana_fraza = "fileThumb";
    char * wsk_szukana_fraza = nullptr;
    int pozycja_wsk = 0;
    int ile_zgodnych = 0;
    int pozycja_szukania = 0;
    
    //ustawienie wskaznika
    wsk_szukana_fraza = (char*)baza_danych->ptr_buffer;

    for(int i = 0 ; i<baza_danych->r_pamieci; i++)
    {
        if(*(wsk_szukana_fraza+i) == szukana_fraza[pozycja_szukania])
        {
            ile_zgodnych++;
            pozycja_szukania++;
        }
        else
        {
            ile_zgodnych = 0;
            pozycja_szukania = 0;
        }
        
        if(ile_zgodnych == 9)
        {
            i = i+11;
            while(*(wsk_szukana_fraza+i) != '\"')
            {
                link = link + *(wsk_szukana_fraza+i);
                i++;
            }
            ile_zgodnych = 0;
            pozycja_szukania = 0;

            //tutaj musze zrobic wyjatek bo cos sie z serwerami dzieje złego dla strony i.4cdn.org
            napis_wyciety = link.substr(0, 10);
    
            if(napis_wyciety != link_zakazany)
            {
                linki.push_back(link);
            }
            

            link.clear();
        }
    }
    
    return linki;
}

excel pobranie_pliku(vector<string> link)
{
    //vector<excel> excel_pliki;
    static int pozycja=0;
    CURLcode res;
    excel Baza_danych;

    CURL * curl =  curl_easy_init();

        //ustawia polaczenie ze stroną
        //zerowanie wskaznikow itd przed wykonaniem pentli (robota na czysto)
    Baza_danych.ptr_buffer = Baza_danych.ptr = nullptr;
    Baza_danych.nowyr_pamieci = Baza_danych.r_pamieci = 0;

    //cout << "plik nr: " << pozycja+1 << " z " << link.size() << endl;

        //ustawienie polaczen do stron z plikami
    curl_easy_setopt(curl, CURLOPT_URL, link[pozycja].c_str() );
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&Baza_danych);

    res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            cout << "Coś poszło nie tak :/ " <<curl_easy_strerror(res) << endl;
        }
    
        curl_easy_reset(curl);
        pozycja++;
                
    
    curl_easy_cleanup(curl);
    return Baza_danych;
}

void zapis_plikow(excel wsk_do_plikow,vector<string> nazwy_plikow,string sciezka)
{
    static int pozycja = 0;
    int static fd = 0;
    string napis_calosciowy;

    napis_calosciowy = sciezka + nazwy_plikow[pozycja];
    fd = open(napis_calosciowy.c_str(), O_RDWR | O_CREAT);
    size_t plik = write(fd, wsk_do_plikow.ptr_buffer, wsk_do_plikow.r_pamieci);

    //ustawaienie do nastepnego przebiegu
    pozycja++;
    fd = 0;

}

vector<string> szukanie_nazwy(vector<string> linki)
{
    string chwilowy;
    vector<string> nazwy_plikow;
    int pozycja_kropek[10]{};
    int index = 0; //index jest odrazu iloscia slaszy w linku
    int dlugosc_napisu =0;
    int pozycja_poczatkowa = 0;

    //szukanie ostatniej kropki
    for(int i=0;i<linki.size();i++)
    {
        //szukanie ostaniego slesha w linku ( przed i po kropce litery beda za nazwe i rozszerzenie)
        for(int y=0;y<linki[i].length();y++)
        {
            if(linki[i][y] == '/')
            {
                pozycja_kropek[index] = y;
                index++;
            }
            //kopiujemy tekst zaraz po ostatnim slashu
        }

        pozycja_poczatkowa = pozycja_kropek[index-1]+1;
        dlugosc_napisu = linki[i].length()-pozycja_kropek[index-1];

        chwilowy =  linki[i];
        nazwy_plikow.push_back(chwilowy.substr(pozycja_poczatkowa, dlugosc_napisu));

        index = 0;
        dlugosc_napisu = 0;
        pozycja_poczatkowa = 0;

    }
    return nazwy_plikow;
}

string sciezka()
{
    int fd = open("/home/Drop/file", O_RDWR | O_TRUNC);
    lseek(fd,0,SEEK_SET);
    char znak = 'a';
    int ile_nowych_lini=0;
    int ilosc_folderow = 0;
    ssize_t ile = 1;

    do
    {
        ile = read(fd, &znak, 1);
        if(znak == '\n')
        {
            ile_nowych_lini++;
        }

    }while(ile > 0);

    ilosc_folderow = ile_nowych_lini -2;
    close(fd);
    string komenda = "cd /home/Drop; mkdir drop" + to_string(ilosc_folderow);
    system(komenda.c_str());
    string sciezka = "/home/Drop/drop" + to_string(ilosc_folderow) + "/";

    return sciezka;
}