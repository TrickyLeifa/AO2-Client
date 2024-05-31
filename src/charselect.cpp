#include "courtroom.h"

#include "aoimage.h"
#include "lobby.h"

void Courtroom::constructCharacterSelect()
{
  if (ui_char_select)
  {
    ui_char_select->hide();
    ui_char_select->deleteLater();
    ui_char_select = nullptr;
  }

  ui_char_select = new kal::CharacterSelect(ao_app, ui_char_select_background);
  ui_char_select->resize(ui_char_select_background->size());
  ui_char_select->setCharacterList(m_character_list);

  connect(ui_char_select, &kal::CharacterSelect::exitServer, this, &Courtroom::exitServer);
  connect(ui_char_select, &kal::CharacterSelect::spectatorSelected, this, &Courtroom::requestSpectatorAsCharacter);
  connect(ui_char_select, &kal::CharacterSelect::characterSelected, this, &Courtroom::requestCharacter);
}

void Courtroom::exitServer()
{
  ao_app->construct_lobby();
  ao_app->destruct_courtroom();
}

void Courtroom::requestSpectatorAsCharacter()
{
  requestCharacter(SpectatorCharacterId);
}

void Courtroom::requestCharacter(int n_char)
{
  if (n_char != m_character_id || n_char == SpectatorCharacterId)
  {
    ao_app->send_server_packet(AOPacket("CC", {QString::number(ao_app->client_id), QString::number(n_char), get_hdid()}));
  }

  if (n_char == m_character_id || n_char == SpectatorCharacterId)
  {
    update_character(n_char);
    enter_courtroom();
  }
}
