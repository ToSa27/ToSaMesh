#include "ToSaMeshMaster.h"

LoggerPtr logger(Logger::getLogger("ToSa.Mesh"));

int main(int argc, const char * argv[]) {
	ConsoleAppender * consoleAppender = new ConsoleAppender(LayoutPtr(new SimpleLayout()));
	consoleAppender->setImmediateFlush(true);
	BasicConfigurator::configure(AppenderPtr(consoleAppender));

	LOG4CXX_INFO(logger, "ToSaMeshMaster starting...");

	ToSaMeshDatabase::init();
	
	MessageQueue mqSubscribe;
	MessageQueue mqTx;
	MessageQueue mqRx;
	MessageQueue mqToUi;
	MessageQueue mqFromUi;
	
	ToSaMeshMessageDispatcher disp(&mqSubscribe, &mqTx, &mqRx, &mqToUi, &mqFromUi);

	ToSaMeshThreadNode tNode;
	ToSaMeshThreadSocket tSocket;
	ToSaMeshThreadTime tTime;
	ToSaMeshThreadDataLogger tDataLogger;

	tNode.setName("Node");
	tSocket.setName("Socket");
	tTime.setName("Time");
	tDataLogger.setName("DataLogger");
	
	tNode.start(&disp);
	tSocket.start(&disp);
	tTime.start(&disp);
	tDataLogger.start(&disp);

//	system("cd ../web; nohup nodejs ToSaMeshWeb.js >> ../log/ToSaMeshWeb.log 2>&1 &");
	
	LOG4CXX_INFO(logger, "ToSaMeshMaster running...");

	tDataLogger.join();	
	tTime.join();
	tSocket.join();
	tNode.join();
}
