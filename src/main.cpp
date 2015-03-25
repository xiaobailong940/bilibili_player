
#include <iostream>
#include <QApplication>
#include <QString>
#include <QScreen>
#include <QMessageBox>

#include "bplayer.hpp"
#include "bilibilires.hpp"

void fuckoff_low_dpi_screen(const QScreen* screen)
{
	std::cout << "screen DPI = " << screen->physicalDotsPerInch() << std::endl;
	std::cout << "screen Logical DPI = " << screen->logicalDotsPerInch() << std::endl;

	if ( screen->logicalDotsPerInch() < screen->physicalDotsPerInch())
	{
		std::cout << "you idiot! stupid dumb! Go fuck you self, have't you see the font tooo small for you?" << std::endl;

		QString msg = QString("你的屏幕实际 DPI 为 %1, 但是系统才设置 %2 的 DPI, 难道你不觉得字体很小看着不舒服么？\n"
		"赶紧打开系统设置，将 DPI 设置的比屏幕 DPI 大点！建议你设置为 %3").arg(screen->physicalDotsPerInch()).arg(screen->logicalDotsPerInch())
		.arg([](qreal phydpi){
			if ( phydpi < 96)
				return 96;
			if ( phydpi < 115)
				return 120;
			if  (phydpi < 130)
				return 144;
			if (phydpi <= 182)
				return 192;
			if (phydpi <= 230)
				return 240;
			if(phydpi <= 260)
				return 288;
			if (phydpi < 330)
				return 384;
			if (phydpi < 500)
				return 576;

			return (int(qRound(phydpi+1))/ 8 + 1) * 8;
		}(screen->physicalDotsPerInch()));

		QMessageBox box;
		box.setText(msg);
		box.exec();
	}

	if( screen->devicePixelRatio() != 1.0)
	{
		std::cout << "do not set devicePixelRatio, you idiot" << std::endl;
// 		std::exit(1);
	}
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QCoreApplication::setApplicationName("bilibili player");
	QCoreApplication::setApplicationVersion("0.9");

	QIcon exe_icon(":/ui/bilibili.ico");

	app.setWindowIcon(exe_icon);

	QCommandLineParser cliparser;
	cliparser.setApplicationDescription("bilibili 播放器");

	cliparser.addHelpOption();
	cliparser.addVersionOption();
	cliparser.addOption({"about-qt", "display about-qt dialog"});
	cliparser.addOption({"about", "display about dialog"});

	cliparser.addOption({"use-bullet", "use bullet engine to manage danmaku"});
	cliparser.addOption({"videourl", "alternative video url, useful for play local video file while still  be able to see danmaku", "uri"});
	cliparser.addOption({"nogl", "do not using opengl to render the video and danmaku"});
	cliparser.addOption({"no-minimalsize", "allow resize freely"});
	cliparser.addOption({"force-aspect", "force video aspect", "16:9"});

	cliparser.process(app);

	fuckoff_low_dpi_screen(app.primaryScreen());

	if (cliparser.isSet("about-qt"))
	{
		app.aboutQt();
		return 0;
	}

	QString bilibili_url;
	// argv[1] should by the url to play

	if (cliparser.positionalArguments().size() >= 1)
	{
		bilibili_url = cliparser.positionalArguments().at(0);
		std::cerr << "play bilibili url: " << bilibili_url.toStdString() << std::endl;

	}else
	{
		std::cerr << "\n\n\n -- 必须要有 bilibili 地址哦！ -- \n 以下是帮助" << std::endl;

		cliparser.showHelp(1);
	}

	BPlayer player;

	if (cliparser.isSet("use-bullet"))
	{
		player.setProperty("UseBullet", cliparser.value("use-bullet") != "no");
	}

	if (cliparser.isSet("nogl"))
	{
		player.setProperty("UseOpenGL", ! (cliparser.value("nogl") != "no"));
	}

	if (cliparser.isSet("force-aspect"))
	{
		player.setProperty("VideoAspect", cliparser.value("force-aspect"));

		qDebug() << "force aspect to :" <<  cliparser.value("force-aspect");
	}

	if (cliparser.isSet("no-minimalsize"))
	{
		player.setProperty("AllowAnySize", (cliparser.value("no-minimalsize") != "no"));
	}

	auto bilibili_res = new BiliBiliRes(bilibili_url.toStdString());

	if (cliparser.isSet("videourl"))
	{
		bilibili_res->setProperty("DoNotExtractVideoUrl", true);

		VideoURL url;
		url.url = cliparser.value("videourl").toStdString();
		player.append_video_url(url);
	}else
	{
		QObject::connect(bilibili_res, SIGNAL(video_url_extracted(VideoURL)), &player, SLOT(append_video_url(VideoURL)));
	}

	QObject::connect(bilibili_res, SIGNAL(barrage_extracted(QDomDocument)), &player, SLOT(set_barrage_dom(QDomDocument)));
	QObject::connect(bilibili_res, SIGNAL(finished()), &player, SLOT(start_play()));
	QObject::connect(bilibili_res, SIGNAL(finished()), bilibili_res, SLOT(deleteLater()));

	app.exec();

	return 0;
}

