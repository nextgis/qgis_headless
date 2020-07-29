#pragma once

#include <QObject>

class TestLib : public QObject
{
    Q_OBJECT
public:
    TestLib(int argc, char **argv, QObject *parent = nullptr);

private slots:
    void initTestCase();
    void testGetVersion();
    void testRenderVector();
    void testRenderRaster();
    void cleanupTestCase();

private:
    int mArgc;
    char **mArgv;
};
