#include "migowy.h"
bool bQuit = false;

//Sygnał zakmnięcia wątka GUI, żeby wszystkie zasoby mogły zostać zwolnione przez zakończeniem programu
void quit_signal(int signal)
{
    bQuit = true;
}

void logic()
{
    std::signal(SIGINT, quit_signal);
     
    while (!bQuit)
    {
        cv::Mat read;
        camera.read(read);

        //imgCv jest potem używany w pętli rysującej
        mut.lock();
        read.copyTo(imgCv);
        mut.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Migowy w;
    //Wczytywanie opcji
    if (boost::filesystem::exists("options.dat"))
    {
        std::ifstream save("options.dat");
        boost::archive::binary_iarchive ia(save);
        ia >> w;
    }
    w.setupGuiAll();
    
    camera = cv::VideoCapture(1);   //1, bo chcemy wczytać kamerę OBS
    //Używamy dwóch pluginów i specjalnej aplikacji, żeby móc transmitować obraz z komórki do tego urządzenia i odbierać go w tym programie
    camera.read(imgCv);

    //Osobny wątek zbierający informacje z kamery, żeby nie czekać na operacje I/O
    guiThread = new std::thread(logic);
    w.show();
    w.setFocus();
    return a.exec();
}

