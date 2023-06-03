#include "EclipseTest.h"
#include "ui_EclipseTest.h"

using namespace cv;

EclipseTest::EclipseTest(QWidget *parent) : QMainWindow(parent), ui(new Ui::EclipseTest)
{
    ui->setupUi(this);

    connect(ui->displayButton, SIGNAL(clicked()), this, SLOT(displayLiveView()));
	connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopLiveView()));

	connect(this, SIGNAL(signal_changeDate(QString)), this, SLOT(slot_changeDate(QString)));

	pthread_create(&threadId, NULL, threadFunction, (void *)this);

	capturePtr = nullptr;
	framePtr   = nullptr;
	timerPtr   = nullptr;

	capturePtr = new VideoCapture(0);
	framePtr   = new Mat();
	timerPtr   = new QTimer(this);

	ui->dateLineEdit->setReadOnly(true);

	//ui->liveViewLineEdit->setText("Live View Status ==> Ready");

} //end of constructor

EclipseTest::~EclipseTest()
{
    delete ui;

    if(capturePtr != nullptr)
	{
		delete capturePtr;
		capturePtr = nullptr;
	}

	if(framePtr != nullptr)
	{
		delete framePtr;
		framePtr = nullptr;
	}

	if(timerPtr != nullptr)
	{
		timerPtr->stop();
		capturePtr->release();

		delete timerPtr;
		timerPtr = nullptr;
	}

} //end of destructor

void EclipseTest::slot_changeDate(QString str)
{
	ui->dateLineEdit->setText(str);

} //end of function slot_changeDate

void *EclipseTest::threadFunction(void *arg)
{
	EclipseTest *eclipseTest = static_cast<EclipseTest *>(arg);

	while(true)
	{
		QDateTime currentDateTime = QDateTime::currentDateTime();
		QString currentDate       = currentDateTime.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");

		eclipseTest->signal_changeDate(currentDate);

		sleep(1);
	}

} //end of function threadFunction

void EclipseTest::displayLiveView()
{
    /*capturePtr = new VideoCapture(0);
    framePtr   = new Mat();*/

    capturePtr->open(0);

    //Disable continuous auto focus (on: 1, off: 0)
    capturePtr->set(CAP_PROP_AUTOFOCUS, 0);

    //Set manual focus step (0 - 255)
    capturePtr->set(CAP_PROP_FOCUS, 200);

    //640กั360 (16:9)
    capturePtr->set(CAP_PROP_FRAME_WIDTH, 640);
    capturePtr->set(CAP_PROP_FRAME_HEIGHT, 360);

    //Frame rate
    capturePtr->set(CAP_PROP_FPS, 30);

    if(capturePtr->isOpened())
    {
        double frameRate = capturePtr->get(CV_CAP_PROP_FPS);

        *capturePtr >> *framePtr;

        if(!framePtr->empty())
        {
            QImage qImage = cvMat2QImage(*framePtr);

            ui->liveViewLabel->setPixmap(QPixmap::fromImage(qImage));

            //timerPtr = new QTimer(this);
            timerPtr->setInterval(1000 / frameRate);
            connect(timerPtr, SIGNAL(timeout()), this, SLOT(nextFrame()));
            timerPtr->start();
        }

        //QMessageBox::information(this, "Live View", "Start");
        //ui->liveViewLineEdit->setText("Live View Status ==> Started");
    }

} //end of function displayLiveView

void EclipseTest::stopLiveView()
{
    timerPtr->stop();

    //QMessageBox::information(this, "Live View", "Stop");
    //ui->liveViewLineEdit->setText("Live View Status ==> Stopped");

    disconnect(timerPtr, SIGNAL(timeout()), this, SLOT(nextFrame()));
    capturePtr->release();

} //end of function stopLiveView

void EclipseTest::nextFrame()
{
    *capturePtr >> *framePtr;

    if(capturePtr->isOpened())
    {
        int width  = 0;
        int height = 0;

        width  = saturate_cast<int>(capturePtr->get(CAP_PROP_FRAME_WIDTH));
        height = saturate_cast<int>(capturePtr->get(CAP_PROP_FRAME_HEIGHT));

        Mat outputFrame(height, width, CV_8UC3);

        //toGrayImage(*framePtr, outputFrame, height, width);
        //toNegativeImage(*framePtr, outputFrame, height, width);

        qImage  = cvMat2QImage(*framePtr);
        //qImage  = cvMat2QImage(outputFrame);
        qPixmap = QPixmap::fromImage(qImage);

        qPixmap = qPixmap.scaled(ui->liveViewLabel->width(), ui->liveViewLabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui->liveViewLabel->setPixmap(qPixmap);
    }

} //end of function nextFrame

void EclipseTest::toNegativeImage(const Mat &sourceImage, Mat &outputImage, const int imageHeight, const int imageWidth)
{
    for(int y = 0; y < imageHeight; y++)
    {
        for(int x = 0; x < imageWidth; x++)
        {
            uchar pixel_B = sourceImage.at<Vec3b>(y, x)[CHANNEL_B];
            uchar pixel_G = sourceImage.at<Vec3b>(y, x)[CHANNEL_G];
            uchar pixel_R = sourceImage.at<Vec3b>(y, x)[CHANNEL_R];

            outputImage.at<Vec3b>(y, x)[CHANNEL_B] = saturate_cast<uchar>(255 - pixel_B);
            outputImage.at<Vec3b>(y, x)[CHANNEL_G] = saturate_cast<uchar>(255 - pixel_G);
            outputImage.at<Vec3b>(y, x)[CHANNEL_R] = saturate_cast<uchar>(255 - pixel_R);
        }
    }

} //end of function toNegativeImage

void EclipseTest::toGrayImage(const Mat &sourceImage, Mat &outputImage, const int imageHeight, const int imageWidth)
{
    for(int y = 0; y < imageHeight; y++)
    {
        for(int x = 0; x < imageWidth; x++)
        {
            uchar pixel_B  = sourceImage.at<Vec3b>(y, x)[CHANNEL_B];
            uchar pixel_G  = sourceImage.at<Vec3b>(y, x)[CHANNEL_G];
            uchar pixel_R  = sourceImage.at<Vec3b>(y, x)[CHANNEL_R];
            float avgPixel = saturate_cast<float>((pixel_B + pixel_G + pixel_R) / 3.0);

            outputImage.at<Vec3b>(y, x)[CHANNEL_B] = saturate_cast<uchar>(avgPixel);
            outputImage.at<Vec3b>(y, x)[CHANNEL_G] = saturate_cast<uchar>(avgPixel);
            outputImage.at<Vec3b>(y, x)[CHANNEL_R] = saturate_cast<uchar>(avgPixel);
        }
    }

} //end of function toGrayImage

QImage EclipseTest::cvMat2QImage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }

} //end of function cvMat2QImage
