#include <QtTest>

class UITests : public QObject
{
    Q_OBJECT

private slots:
    void testPlaceholder();
};

void UITests::testPlaceholder()
{
    QVERIFY(true); // Placeholder test
}

QTEST_MAIN(UITests)
#include "UITests.moc"