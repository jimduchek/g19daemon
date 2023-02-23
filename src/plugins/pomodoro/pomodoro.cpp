/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pomodoro.hpp"
#include "../../g19daemon.hpp"
#include "../../gscreen.hpp"
#include <QImage>
#include <QPainter>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QtCore>
#include <QPainterPath>
#include <QtSvg/QSvgRenderer>



Pomodoro::Pomodoro() {
  Q_INIT_RESOURCE(pomodoro);

  isActive = false;
  screen = new Gscreen(QImage(":/pomodoro/icon.png"), tr("Pomodoro"));

  startImage = QIcon(":/pomodoro/start").pixmap(QSize(32,32)).toImage();
  pauseImage = QIcon(":/pomodoro/pause").pixmap(QSize(32,32)).toImage();
  stopImage = QIcon(":/pomodoro/stop").pixmap(QSize(32,32)).toImage();
  backwardImage = QIcon(":/pomodoro/backward").pixmap(QSize(32,32)).toImage();
  forwardImage = QIcon(":/pomodoro/forward").pixmap(QSize(32,32)).toImage();


  state = STATE_STOPPED;
  resetPomodoros();

  
  //trayIconMenu = new QMenu(this);
  //trayIconMenu->addAction(tr("Show"), this, SLOT(pause()));
  //trayIconMenu->addAction(tr("Reset LCD Backlight"), this,
  //                          SLOT(resetLcdBacklight()));
  //trayIconMenu->addAction(QIcon(":/off.png"), "&Quit", this, SLOT(quit()));
  
  QPixmap pix = QPixmap(48,48);
  QIcon miniIcon = QIcon(pix);
  trayIcon = new QSystemTrayIcon(this);
  //trayIcon->setContextMenu(trayIconMenu);
  trayIcon->setToolTip("G19 Pomodoro");
  trayIcon->setIcon(miniIcon);
  trayIcon->show();
  connect(trayIcon, &QSystemTrayIcon::activated, this, &Pomodoro::iconActivated);



  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, QOverload<>::of(&Pomodoro::tick));
  timer->start(250);
}

Pomodoro::~Pomodoro() { delete screen; }

QString Pomodoro::getName() { return tr("Pomodoro"); }

void Pomodoro::resetPomodoros() {
  timeLeft = fullTime = 25*60*1000;
  shortBreaksLeft = 3;
  isPaused = true;
  lastTick = QTime::currentTime();
}

void Pomodoro::pause() {}
void Pomodoro::next() {}
void Pomodoro::previous() {}
void Pomodoro::resume() {}
void Pomodoro::stop() {}

void Pomodoro::iconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (state == STATE_STOPPED) {
    switchState(false, STATE_POMODORO);
  } else {
    switchState(!isPaused, state);
  }
}


void Pomodoro::lKeys(int keys) {
  if (isActive) {
    if (keys & G19_KEY_LRIGHT) {
      if (state != STATE_STOPPED) {
        timeLeft = 0;
      }
    }

    if (keys & G19_KEY_LLEFT) {
      switch(state) {
      case STATE_POMODORO:
        timeLeft = fullTime = 25 * 60 * 1000;
        break;
      case STATE_LONG_BREAK:
        timeLeft = fullTime = 20 * 60 * 1000;
        break;
      case STATE_SHORT_BREAK:
        timeLeft = fullTime = 5 * 60 * 1000;
        break;
      case STATE_STOPPED:
      default:
        break;
     }
    }

    if (keys & G19_KEY_LDOWN) {
    }

    if (keys & G19_KEY_LOK) {
      if (state == STATE_STOPPED) {
        switchState(false, STATE_POMODORO);
      } else {
        switchState(!isPaused, state);
      }
    }

    if (keys & G19_KEY_LMENU) {
      if (state != STATE_STOPPED) {
        switchState(true, STATE_STOPPED);
      }
    }

    tick();
  } 
}

void Pomodoro::mKeys(int keys) {}

void Pomodoro::setActive(bool active) {
  isActive = active;

  if (active) {
    paint();
  }
}

void Pomodoro::switchState(bool pause, PomodoroState s) {
  QString program = "/home/jrduchek/tools/bin/pomodoro.sh";
  QStringList arguments;
  QProcess *myProcess = new QProcess();


  isPaused = pause;

  switch(s) {
    case STATE_POMODORO:
      arguments << "pomodoro";
      break;
    case STATE_LONG_BREAK:
      arguments << "long_break";
      break;
    case STATE_SHORT_BREAK:
      arguments << "short_break";
      break;
    case STATE_STOPPED:
      arguments << "stopped";
      break;        
  }

  if (state != s) {
    arguments << "starting";
    switch(s) {
      case STATE_POMODORO:
        timeLeft = fullTime = 25 * 60 * 1000;
        break;
      case STATE_LONG_BREAK:
        shortBreaksLeft = 3;
        timeLeft = fullTime = 20 * 60 * 1000;
        break;
      case STATE_SHORT_BREAK:
        shortBreaksLeft--;
        timeLeft = fullTime = 5 * 60 * 1000;
        break;
      case STATE_STOPPED:
        resetPomodoros();
        break;        
    }
    state = s;
  } else {
    if (isPaused) {
      arguments << "paused";
    } else { 
      arguments << "resumed";
    }
  }
  myProcess->start(program, arguments);
}

void Pomodoro::tick() {
  QTime now = QTime::currentTime();
  int time_diff = lastTick.msecsTo(now);
  lastTick = now;

  if (isPaused || state == STATE_STOPPED) { // don't do nothing


  } else {
    timeLeft -= time_diff;

    if (timeLeft <= 0) {
      switch(state) {
        case STATE_POMODORO:
          if (shortBreaksLeft <= 0) {
            switchState(isPaused, STATE_LONG_BREAK);
          } else {
            switchState(isPaused, STATE_SHORT_BREAK);
          }
          break;
        case STATE_LONG_BREAK:
          switchState(isPaused, STATE_POMODORO);
          break;
        case STATE_SHORT_BREAK:
          switchState(isPaused, STATE_POMODORO);
          break;
      }
    }


  }

  paint();
}

void Pomodoro::paintTrayIcon() {
  QPixmap pix = QPixmap(48,48);
  pix.fill(Qt::transparent);
  QPainter *p = new QPainter(&pix);

  p->setRenderHint(QPainter::Antialiasing);

  if (state != STATE_STOPPED) {
    /*
    QFont stateFont("DejaVuSans", 14);
    QString s;

    p->setFont(stateFont);
    switch (state) {
    case STATE_POMODORO:
      s = "Pomodoro";
      break;
    case STATE_LONG_BREAK:
      s = "Long Break";
      break;
    case STATE_SHORT_BREAK:
      s = "Break";
      break;
    }

    if (isPaused) {
      p->setPen(qRgb(128, 128, 128));
    } else {
      p->setPen(qRgb(255, 255, 255));
    }    p->drawText(90, 65, 140, 25, Qt::AlignCenter, s);
    */

    QFont timemin("DejaVuSans", 12);
    QFont timesec("DejaVuSans", 8);

    int minutes = timeLeft / (60 * 1000);
    int seconds = (timeLeft % (60 * 1000)) / 1000;

    QString s_min = QString::number(minutes).rightJustified(2, '0');
    QString s_sec = QString::number(seconds).rightJustified(2, '0');

    if (isPaused) {
      p->setPen(qRgb(192, 192, 192));
    } else {
      p->setPen(qRgb(48, 48, 48));
    }

    p->setFont(timemin);

    p->drawText(5, 12, 38, 16, Qt::AlignCenter, s_min);
    p->setFont(timesec);
    p->drawText(5, 28, 38, 14, Qt::AlignCenter, s_sec);
    

    qreal percent = (qreal)timeLeft/(qreal)fullTime;
    qreal pd = percent * 360;
    qreal rd = 360 - pd;

    QPainterPath path, path2;
    path.moveTo(24, 4);
    path.arcTo(QRectF(4, 4, 42, 42), 90, pd);
    QPen pen, pen2;
    pen.setCapStyle(Qt::FlatCap);
    if (isPaused) {
      pen.setColor(QColor("#b7b7b7"));
    } else {
      pen.setColor(QColor("#30b7e0"));
    }
    pen.setWidth(4);
    p->strokePath(path, pen);
    path2.moveTo(24, 4);
    pen2.setWidth(4);
    pen2.setColor(QColor("#606060"));
    pen2.setCapStyle(Qt::FlatCap);
    path2.arcTo(QRectF(4, 4, 42, 42), 90, -rd);
    p->strokePath(path2, pen2);

  } else {
    QPainterPath path;
    path.moveTo(24, 4);
    path.arcTo(QRectF(4, 4, 42, 42), 90, 360.0);
    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(QColor("#b7b7b7"));
    pen.setWidth(4);
    p->strokePath(path, pen);

    // This is not centered... but it _looks_ centered.

    QImage image = QIcon(":/pomodoro/start").pixmap(QSize(48, 48)).toImage();
    QPainter p2(&image);
    p2.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    p2.fillRect(QRectF(0,0,48,48), QColor("#b7b7b7"));
    p2.end();
    p->drawImage(QRectF(12, 10, 32, 32), image);
  }

  delete p;
  QIcon miniIcon = QIcon(pix);
  trayIcon->setIcon(miniIcon);

}

void Pomodoro::paint() {
  paintTrayIcon();


  if (!isActive) {
    return;
  }

  QPainter *p;
  int side = qMin(320, 240);
  p = screen->beginFullScreen();

  p->setRenderHint(QPainter::Antialiasing);

  if (state != STATE_STOPPED) {

    QFont stateFont("DejaVuSans", 14);
    QString s;

    p->setFont(stateFont);
    switch (state) {
    case STATE_POMODORO:
      s = "Pomodoro";
      break;
    case STATE_LONG_BREAK:
      s = "Long Break";
      break;
    case STATE_SHORT_BREAK:
      s = "Break";
      break;
    }

    if (isPaused) {
      p->setPen(qRgb(128, 128, 128));
    } else {
      p->setPen(qRgb(255, 255, 255));
    }    p->drawText(90, 65, 140, 25, Qt::AlignCenter, s);

    QFont time("DejaVuSans", 20);
    p->setFont(time);

    int minutes = timeLeft / (60 * 1000);
    int seconds = (timeLeft % (60 * 1000)) / 1000;

    s = QString::number(minutes) + ":" +
        QString::number(seconds).rightJustified(2, '0');

    if (isPaused) {
      p->setPen(qRgb(128, 128, 128));
    } else {
      p->setPen(qRgb(255, 255, 255));
    }
    p->drawText(90, 105, 140, 30, Qt::AlignCenter, s);


    qreal percent = (qreal)timeLeft/(qreal)fullTime;
    qreal pd = percent * 360;
    qreal rd = 360 - pd;

    QPainterPath path, path2;
    path.moveTo(160, 5);
    path.arcTo(QRectF(40, 5, 240, 230), 90, pd);
    QPen pen, pen2;
    pen.setCapStyle(Qt::FlatCap);
    if (isPaused) {
      pen.setColor(QColor("#b7b7b7"));
    } else {
      pen.setColor(QColor("#30b7e0"));
    }
    pen.setWidth(10);
    p->strokePath(path, pen);
    path2.moveTo(160, 5);
    pen2.setWidth(10);
    pen2.setColor(QColor("#606060"));
    pen2.setCapStyle(Qt::FlatCap);
    path2.arcTo(QRectF(40, 5, 240, 230), 90, -rd);
    p->strokePath(path2, pen2);

    p->drawImage(QRectF(90, 150, 32, 32), backwardImage);
    if (isPaused) {
      p->drawImage(QRectF(144, 150, 32, 32), startImage);
    } else {
      p->drawImage(QRectF(144, 150, 32, 32), pauseImage);
    }
    p->drawImage(QRectF(198, 150, 32, 32), forwardImage);

    p->drawImage(QRectF(16, 192, 32, 32), stopImage);
  } else {
    QPainterPath path;
    path.moveTo(160, 5);
    path.arcTo(QRectF(40, 5, 240, 230), 90, 360.0);
    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(QColor("#b7b7b7"));
    pen.setWidth(10);
    p->strokePath(path, pen);

    // This is not centered... but it _looks_ centered.
    QImage start = QIcon(":/pomodoro/start").pixmap(QSize(160,160)).toImage();
    p->drawImage(QRectF(90, 40, 160, 160), start);
  }
  screen->end();
  emit doAction(displayFullScreen, screen);
}

bool Pomodoro::isPopup() { return false; }

QObject *Pomodoro::getQObject() { return this; }

QImage Pomodoro::getIcon() { return QImage(":/pomodoro/menu_icon.png"); }
