#ifndef STEPPER_H
#define STEPPER_H

#include <QObject>

class stepper : public QObject
{
    Q_OBJECT
public:
    explicit stepper(QObject *parent = nullptr);

signals:

public slots:
};

#endif // STEPPER_H