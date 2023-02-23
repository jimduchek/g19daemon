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

#ifndef POMODORO_H
#define POMODORO_H

#include "../../gscreen.hpp"
#include "../../plugininterface.hpp"
#include <QtCore>
#include <QtPlugin>
#include <QMenu>
#include <QSystemTrayIcon>

enum PomodoroState {
  STATE_STOPPED,
  STATE_POMODORO,
  STATE_SHORT_BREAK,
  STATE_LONG_BREAK,
};

class Pomodoro : public QObject, public PluginInterface {
  Q_OBJECT
  Q_INTERFACES(PluginInterface)
  Q_PLUGIN_METADATA(IID "pomodoro")

public:
  Pomodoro();
  ~Pomodoro();
  void lKeys(int keys);
  QString getName();
  QImage getIcon();
  void setActive(bool active);
  bool isPopup();
  QObject *getQObject();
  void mKeys(int keys);

public slots:
  void pause();
  void next();
  void previous();
  void resume();
  void stop();
  void iconActivated(QSystemTrayIcon::ActivationReason reason);


private:
  Gscreen *screen;
  bool isActive;
  bool isPaused;
  int timeLeft;
  int fullTime;
  int shortBreaksLeft;
  void paint();
  void tick();
  void switchState(bool pause, PomodoroState s);
  void resetPomodoros();
  QTime lastTick;
  PomodoroState state;

  QMenu *trayIconMenu;
  QSystemTrayIcon *trayIcon;

  QImage startImage;
  QImage pauseImage;
  QImage stopImage;
  QImage backwardImage;
  QImage forwardImage;

  void paintTrayIcon();
signals:
  void doAction(gAction action, void *data); // Signal to draw img on screen
};

#endif // POMODORO_H
