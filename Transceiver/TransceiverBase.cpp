#include "TransceiverBase.hpp"

#include <exception>

#include <QString>
#include <QTimer>
#include <QThread>
#include <QDebug>

#include "moc_TransceiverBase.cpp"

namespace
{
  auto const unexpected = TransceiverBase::tr ("Unexpected rig error");
}

void TransceiverBase::start (unsigned sequence_number) noexcept
{
  CAT_TRACE ("#: " << sequence_number);

  QString message;
  try
    {
      last_sequence_number_ = sequence_number;
      may_update u {this, true};
      shutdown ();
      startup ();
    }
  catch (std::exception const& e)
    {
      CAT_TRACE ("#: " << sequence_number << " what: " << e.what ());
      message = e.what ();
    }
  catch (...)
    {
      CAT_TRACE ("#: " << sequence_number);
      message = unexpected;
    }
  if (!message.isEmpty ())
    {
      offline (message);
    }
}

void TransceiverBase::set (TransceiverState const& s,
                           unsigned sequence_number) noexcept
{
  CAT_TRACE ("#: " << s);

  QString message;
  try
    {
      last_sequence_number_ = sequence_number;
      may_update u {this, true};
      bool was_online {requested_.online ()};
      if (!s.online () && was_online)
        {
          shutdown ();
        }
      else if (s.online () && !was_online)
        {
          shutdown ();
          startup ();
        }
      if (requested_.online ())
        {
          bool ptt_on {false};
          bool ptt_off {false};
          if (s.ptt () != requested_.ptt ())
            {
              ptt_on = s.ptt ();
              ptt_off = !s.ptt ();
            }
          if (ptt_off)
            {
              do_ptt (false);
              do_post_ptt (false);
              QThread::msleep (100); // some rigs cannot process CAT
                                     // commands while switching from
                                     // Tx to Rx
            }
          if (s.frequency ()    // ignore bogus zero frequencies
              && ((s.frequency () != requested_.frequency () // and QSY
                   || (s.mode () != UNK && s.mode () != requested_.mode ())))) // or mode change
            {
              do_frequency (s.frequency (), s.mode (), ptt_off);
              do_post_frequency (s.frequency (), s.mode ());

              // record what actually changed
              requested_.frequency (actual_.frequency ());
              requested_.mode (actual_.mode ());
            }
          if (!s.tx_frequency ()
              || (s.tx_frequency () > 10000 // ignore bogus startup values
                  && s.tx_frequency () < std::numeric_limits<Frequency>::max () - 10000))
            {
              if ((s.tx_frequency () != requested_.tx_frequency () // and QSY
                   || (s.mode () != UNK && s.mode () != requested_.mode ())) // or mode change
                  // || s.split () != requested_.split ())) // or split change
                  || (s.tx_frequency () && ptt_on)) // or about to tx split
                {
                  do_tx_frequency (s.tx_frequency (), s.mode (), ptt_on);
                  do_post_tx_frequency (s.tx_frequency (), s.mode ());

                  // record what actually changed
                  requested_.tx_frequency (actual_.tx_frequency ());
                  requested_.split (actual_.split ());
                }
            }
          if (ptt_on)
            {
              do_ptt (true);
              do_post_ptt (true);
              QThread::msleep (100); // some rigs cannot process CAT
                                     // commands while switching from
                                     // Rx to Tx
            }

          // record what actually changed
          requested_.ptt (actual_.ptt ());
        }
    }
  catch (std::exception const& e)
    {
      CAT_TRACE ("#: " << sequence_number << " what: " << e.what ());
      message = e.what ();
    }
  catch (...)
    {
      CAT_TRACE ("#: " << sequence_number << " " << sequence_number);
      message = unexpected;
    }
  if (!message.isEmpty ())
    {
      offline (message);
    }
}

void TransceiverBase::startup ()
{
  CAT_TRACE ("startup");
  QString message;
  try
    {
      actual_.online (true);
      requested_.online (true);
      auto res = do_start ();
      do_post_start ();
      Q_EMIT resolution (res);
    }
  catch (std::exception const& e)
    {
      CAT_TRACE ("startup" << " what: " << e.what ());
      message = e.what ();
    }
  catch (...)
    {
      CAT_TRACE ("startup");
      message = unexpected;
    }
  if (!message.isEmpty ())
    {
      offline (message);
    }
}

void TransceiverBase::shutdown ()
{
  CAT_TRACE ("shutdown");
  may_update u {this};
  if (requested_.online ())
    {
      try
        {
          // try and ensure PTT isn't left set
          do_ptt (false);
          do_post_ptt (false);
          if (requested_.split ())
            {
              // try and reset split mode
              do_tx_frequency (0, UNK, true);
              do_post_tx_frequency (0, UNK);
            }
          do_stop ();
          do_post_stop ();
        }
      catch (...)
        {
          CAT_TRACE ("shutdown");
          // don't care about exceptions
        }
    }
  actual_ = TransceiverState {};
  requested_ = TransceiverState {};
}

void TransceiverBase::stop () noexcept
{
  CAT_TRACE ("stop");
  QString message;
  try
    {
      shutdown ();
    }
  catch (std::exception const& e)
    {
      CAT_TRACE ("stop" << " what: " << e.what ());
      message = e.what ();
    }
  catch (...)
    {
      CAT_TRACE ("stop");
      message = unexpected;
    }
  if (!message.isEmpty ())
    {
      offline (message);
    }
  else
    {
      Q_EMIT finished ();
    }
}

void TransceiverBase::update_rx_frequency (Frequency rx)
{
  CAT_TRACE ("frequency: " << rx);
  if (rx)
    {
      actual_.frequency (rx);
      requested_.frequency (rx);    // track rig changes
    }
}

void TransceiverBase::update_other_frequency (Frequency tx)
{
  CAT_TRACE ("frequency: " << tx);
  actual_.tx_frequency (tx);
}

void TransceiverBase::update_split (bool state)
{
  CAT_TRACE ("state: " << state);
  actual_.split (state);
}

void TransceiverBase::update_mode (MODE m)
{
  CAT_TRACE ("mode: " << m);
  actual_.mode (m);
  requested_.mode (m);    // track rig changes
}

void TransceiverBase::update_PTT (bool state)
{
  CAT_TRACE ("state: " << state);
  actual_.ptt (state);
}

void TransceiverBase::update_complete (bool force_signal)
{
  CAT_TRACE ("force signal: " << force_signal);
  if ((do_pre_update ()
       && actual_ != last_)
      || force_signal)
    {
      Q_EMIT update (actual_, last_sequence_number_);
      last_ = actual_;
    }
}

void TransceiverBase::offline (QString const& reason)
{
  CAT_TRACE ("reason: " << reason);
  Q_EMIT failure (reason);
  try
    {
      shutdown ();
    }
  catch (...)
    {
      CAT_TRACE ("reason: " << reason);
      // don't care
    }
}
