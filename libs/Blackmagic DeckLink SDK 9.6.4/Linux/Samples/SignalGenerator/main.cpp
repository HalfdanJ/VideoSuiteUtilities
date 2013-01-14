#include <QApplication>

#include "SignalGenerator.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	SignalGenerator cp;
	cp.setup();
	return app.exec();
}
