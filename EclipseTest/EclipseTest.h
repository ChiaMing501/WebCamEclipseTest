#ifndef ECLIPSETEST_H
#define ECLIPSETEST_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QImage>
#include <QString>
#include <QDateTime>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/legacy/constants_c.h>

#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class EclipseTest; }
QT_END_NAMESPACE

typedef enum
{
    CHANNEL_B = 0,
    CHANNEL_G,
    CHANNEL_R

} ColorChannel;

class EclipseTest : public QMainWindow
{
    Q_OBJECT

	public:
		EclipseTest(QWidget *parent = nullptr);
		~EclipseTest();

	private:
		Ui::EclipseTest *ui;

		cv::VideoCapture *capturePtr;
		cv::Mat          *framePtr;
		QTimer           *timerPtr;
		QImage           qImage;
		QPixmap          qPixmap;
		pthread_t        threadId;

		QImage cvMat2QImage(const cv::Mat &mat);
		void toGrayImage(const cv::Mat &sourceImage, cv::Mat &outputImage, const int imageHeight, const int imageWidth);
		void toNegativeImage(const cv::Mat &sourceImage, cv::Mat &outputImage, const int imageHeight, const int imageWidth);

	private slots:
		void displayLiveView();
	    void stopLiveView();
	    void nextFrame();
	    void slot_changeDate(QString str);

	    static void *threadFunction(void *arg);

	signals:
		void signal_changeDate(QString str);

}; //end of class EclipseTest

#endif // ECLIPSETEST_H
