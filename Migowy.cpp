#include "migowy.h"

Migowy::Migowy(QWidget* parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    loadSigns();
    createErrorMatrix();

    unsigned gridArea = gridSize.width * gridSize.height;
    for (int i = 0; i != gridArea; ++i)
    {
        drawArea.push_back(new QLabel(ui.centralWidget));
        drawImages.emplace_back();
    }
    calibrationContourLoad();
}

Migowy::~Migowy()
{
    std::raise(SIGINT);
    if (guiThread && guiThread->joinable()) guiThread->join();
    delete guiThread;

    for (int i = 0; i != gridSize.width * gridSize.height; ++i)
        delete drawArea[i];
}

void Migowy::resizeEvent(QResizeEvent* event)
{
    QSize drawSpace = ui.centralWidget->size() - QSize(ui.centralWidget->pos().x(), ui.centralWidget->pos().y()) - QSize(0, ui.Options->totalMinimumSize().height());
    size = cv::Size(drawSpace.width() / gridSize.width, drawSpace.height() / gridSize.height);

    ui.drawSpacer->changeSize(0, drawSpace.height(), QSizePolicy::Fixed, QSizePolicy::Fixed);

    int space   = ui.Options->spacing() << 3, // * 8
        margin  = ui.Options->contentsMargins().left() + ui.Options->contentsMargins().right(),
        lines   = 3 * 3,//ui.Options1to2Separator->width(),
        width   = (drawSpace.width() - space - margin - lines) / 5 - 1; //- 1, bo QSlider z jakiegoś powodu jest na pozycji 1

    ui.listWidget               ->setFixedWidth(width);     //ListWidget
    ui.downsizeScaleSlider      ->setFixedWidth(width);     //Options 1
    ui.skinHThresholdLowSlider  ->setFixedWidth(width);     //Options 2
    ui.approxKernelSlider       ->setFixedWidth(width);     //Options 3
    ui.whiteBalanceRedSlider    ->setFixedWidth(width);     //Options 4
    ui.Menu->update();
    QMainWindow::resizeEvent(event);
}

void Migowy::draw(cv::Mat &data, unsigned short slot = 0, QImage::Format format = QImage::Format_BGR888)
{
    static unsigned gridArea = gridSize.width * gridSize.height;

    cv::Mat resized;
    if (slot >= gridArea)
        throw "error";

    cv::resize(data, resized, size);
    QImage imgQt = QImage((uchar*)resized.data, resized.cols, resized.rows, resized.step, format);

    drawArea[slot]->setPixmap(QPixmap::fromImage(imgQt));
    drawArea[slot]->setGeometry(size.width * (slot % gridSize.width), size.height * (slot / gridSize.width), size.width, size.height);
}

void Migowy::clear(unsigned short slot = 0)
{
    static unsigned gridArea = gridSize.width * gridSize.height;

    if (slot >= gridArea)
        throw "error";

    drawArea[slot]->clear();
}

void Migowy::paintEvent(QPaintEvent* event)
 {
    QMainWindow::paintEvent(event);
    resetFilters();
    if (!bDoNotDraw)
    for (unsigned i = 0; i < ui.listWidget->count(); ++i)
    {
        QListWidgetItem* item = ui.listWidget->item(i);
        if (item->flags() & Qt::ItemFlag::ItemIsEnabled)
        {
            if (!filter(properIndex, i, item->text()))
            {
                clear(i);
                item->setTextColor(QColor::fromRgba(qRgb(255, 0, 0)));
            }
            else
            {
                item->setTextColor(QColor());
                draw(drawImages[i], i, (bBlack || bGreyOnce) ? QImage::Format_Grayscale8 : QImage::Format_BGR888);
                bGreyOnce = false;
            }
        }
        else
        {
            clear(i);
            item->setTextColor(QColor::fromRgba(qRgb(0x7F, 0x7F, 0x7F)));
        }
    }
}

void Migowy::calibrationContourLoad()
{
    cv::Mat img = cv::imread("calibrate.png", cv::IMREAD_GRAYSCALE);
    calibrationContourSize = img.size();
    cv::threshold(img, img, 0, 255, cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    calibrationContour = contours[0];
}