#include "testlib.h"

#include <QString>
#include <QTest>

#include <lib.h>

TestLib::TestLib(int argc, char **argv, QObject *p)
    : QObject(p)
    , mArgc(argc)
    , mArgv(argv)
{

}

void TestLib::initTestCase()
{
    HeadlessRender::init(mArgc, mArgv);
}

void TestLib::testGetVersion()
{
    QVERIFY(!QString(HeadlessRender::getVersion()).isEmpty());
}

void TestLib::testRenderVector()
{
    auto image = HeadlessRender::renderVector("", "", 0.0, 0.0, 100.0, 100.0, 800, 600, 4326);
    QVERIFY(image->getData() != nullptr);
    QVERIFY(image->getSize() > 0);
}

void TestLib::cleanupTestCase()
{
    HeadlessRender::deinit();
}
