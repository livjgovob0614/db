// CopyDialog.h

#ifndef __COPY_DIALOG_H
#define __COPY_DIALOG_H

#include <ComboBox.h>
#include <Dialog.h>
#include "CopyDialogRes.h"

const int kCopyDialog_NumInfoLines = 11;

class CCopyDialog: public NWindows::NControl::CModalDialog
{
  NWindows::NControl::CComboBox _path;
  virtual void OnOK();
  virtual bool OnInit();
  virtual bool OnSize(WPARAM wParam, int xSize, int ySize);
  void OnButtonSetPath();
  bool OnButtonClicked(int buttonID, HWND buttonHWND);
public:
  UString Title;
  UString Static;
  UString Value;
  UString Info;
  UStringVector Strings;

  INT_PTR Create(HWND parentWindow = 0) { return CModalDialog::Create(IDD_COPY, parentWindow); }
};

#endif
