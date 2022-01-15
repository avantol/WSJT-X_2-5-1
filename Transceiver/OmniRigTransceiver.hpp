#ifndef OMNI_RIG_TRANSCEIVER_HPP__
#define OMNI_RIG_TRANSCEIVER_HPP__

#include <memory>

#include <QScopedPointer>
#include <QString>
#include <QTimer>

#include "TransceiverFactory.hpp"
#include "TransceiverBase.hpp"

#include "OmniRig.h"

//
// OmniRig Transceiver Interface
//
// Implemented as a Transceiver decorator  because we may want the PTT
// services of another Transceiver  type such as the HamlibTransceiver
// which can  be enabled by wrapping  a HamlibTransceiver instantiated
// as a "Hamlib Dummy" transceiver in the Transceiver factory method.
//
class OmniRigTransceiver final
  : public TransceiverBase
{
  Q_OBJECT;

public:
  static void register_transceivers (logger_type *, TransceiverFactory::Transceivers *, unsigned id1, unsigned id2);

  enum RigNumber {One = 1, Two};

  // takes ownership of wrapped Transceiver
  explicit OmniRigTransceiver (logger_type *, std::unique_ptr<TransceiverBase> wrapped,
                               RigNumber, TransceiverFactory::PTTMethod ptt_type,
                               QString const& ptt_port, QObject * parent = nullptr);
  ~OmniRigTransceiver ();

  int do_start () override;
  void do_stop () override;
  void do_frequency (Frequency, MODE, bool no_ignore) override;
  void do_tx_frequency (Frequency, MODE, bool no_ignore) override;
  void do_mode (MODE) override;
  void do_ptt (bool on) override;

private:
  Q_SLOT void handle_COM_exception (int,  QString, QString, QString);
  Q_SLOT void handle_visible_change ();
  Q_SLOT void handle_rig_type_change (int rig_number);
  Q_SLOT void handle_status_change (int rig_number);
  Q_SLOT void handle_params_change (int rig_number, int params);
  Q_SLOT void handle_custom_reply (int, QVariant const& command, QVariant const& reply);

  MODE map_mode (OmniRig::RigParamX param);
  OmniRig::RigParamX map_mode (MODE mode);

  std::unique_ptr<TransceiverBase> wrapped_; // may be null
  bool use_for_ptt_;
  TransceiverFactory::PTTMethod ptt_type_;
  QScopedPointer<OmniRig::OmniRigX> omni_rig_;
  RigNumber rig_number_;
  QScopedPointer<OmniRig::RigX> rig_;
  QScopedPointer<OmniRig::PortBits> port_;
  QString rig_type_;
  int readable_params_;
  int writable_params_;
  QScopedPointer<QTimer> offline_timer_;
  bool send_update_signal_;
  bool reversed_;   // some rigs can reverse VFOs
};

#endif
