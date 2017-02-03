#include "aomovie.h"

#include "file_functions.h"
#include "courtroom.h"
#include "misc_functions.h"

AOMovie::AOMovie(QWidget *p_parent, AOApplication *p_ao_app) : QLabel(p_parent)
{
  ao_app = p_ao_app;

  m_movie = new QMovie();

  this->setMovie(m_movie);

  connect(m_movie, SIGNAL(frameChanged(int)), this, SLOT(frame_change(int)));
}

void AOMovie::play(QString p_gif, QString p_char)
{
  play_once = true;

  m_movie->stop();

  QString default_path = ao_app->get_default_theme_path() + p_gif + ".gif";
  QString gif_path;

  if (p_gif == "custom")
    gif_path = ao_app->get_character_path(p_char) + "custom.gif";
  else
    gif_path = ao_app->get_theme_path() + p_gif + ".gif";


  if (file_exists(gif_path))
    m_movie->setFileName(gif_path);
  else
    m_movie->setFileName(default_path);

  this->show();
  m_movie->start();
}

void AOMovie::play(QString p_gif, bool p_play_once)
{
  play_once = p_play_once;

  m_movie->stop();

  QString default_path = ao_app->get_default_theme_path() + p_gif + ".gif";
  QString gif_path = ao_app->get_theme_path() + p_gif + ".gif";

  if (file_exists(gif_path))
    m_movie->setFileName(gif_path);
  else
    m_movie->setFileName(default_path);

  this->show();
  m_movie->start();
}

void AOMovie::stop()
{
  m_movie->stop();
  this->hide();
}

void AOMovie::frame_change(int n_frame)
{
  if (n_frame == (m_movie->frameCount() - 1) && play_once)
  {
    //we need this or else the last frame wont show
    delay(m_movie->nextFrameDelay());

    this->stop();

    //signal connected to courtroom object, let it figure out what to do
    done();
  }
}

void AOMovie::combo_resize(int w, int h)
{
  QSize f_size(w, h);
  this->resize(f_size);
  m_movie->setScaledSize(f_size);
}
