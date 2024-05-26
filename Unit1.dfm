object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Form1'
  ClientHeight = 637
  ClientWidth = 966
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object Button1: TButton
    Left = 32
    Top = 24
    Width = 121
    Height = 49
    Caption = #1047#1072#1075#1088#1091#1079#1080#1090#1100' '#1090#1072#1073#1083#1080#1094#1091
    TabOrder = 0
    OnClick = Button1Click
  end
  object VirtualStringTree1: TVirtualStringTree
    Left = 8
    Top = 112
    Width = 481
    Height = 529
    DefaultNodeHeight = 19
    Header.AutoSizeIndex = 0
    Header.Height = 15
    Header.MainColumn = -1
    TabOrder = 1
    OnAddToSelection = VirtualStringTree1AddToSelection
    OnGetText = VirtualStringTree1GetText
    Touch.InteractiveGestures = [igPan, igPressAndTap]
    Touch.InteractiveGestureOptions = [igoPanSingleFingerHorizontal, igoPanSingleFingerVertical, igoPanInertia, igoPanGutter, igoParentPassthrough]
    Columns = <>
  end
  object Button2: TButton
    Left = 312
    Top = 24
    Width = 177
    Height = 49
    Caption = #1059#1076#1072#1083#1080#1090#1100' '#1074#1099#1073#1088#1072#1085#1085#1091#1102' '#1079#1072#1087#1080#1089#1100
    TabOrder = 2
    OnClick = Button2Click
  end
  object VirtualStringTree2: TVirtualStringTree
    Left = 536
    Top = 112
    Width = 400
    Height = 529
    DefaultNodeHeight = 19
    Header.AutoSizeIndex = -1
    Header.Height = 15
    Header.MainColumn = -1
    TabOrder = 3
    OnGetText = VirtualStringTree2GetText
    Touch.InteractiveGestures = [igPan, igPressAndTap]
    Touch.InteractiveGestureOptions = [igoPanSingleFingerHorizontal, igoPanSingleFingerVertical, igoPanInertia, igoPanGutter, igoParentPassthrough]
    Columns = <>
  end
  object Button3: TButton
    Left = 536
    Top = 25
    Width = 400
    Height = 49
    Caption = #1047#1072#1087#1086#1083#1085#1080#1090#1100' '#1080' '#1074#1099#1074#1077#1089#1090#1080' '#1082#1086#1085#1090#1077#1081#1085#1077#1088
    TabOrder = 4
    OnClick = Button3Click
  end
  object ProgressBar1: TProgressBar
    Left = 536
    Top = 80
    Width = 400
    Height = 17
    TabOrder = 5
  end
  object FileOpenDialog1: TFileOpenDialog
    FavoriteLinks = <>
    FileName = 'C:\Users\m1rby\Documents\Embarcadero\Studio\Projects\'#1053#1086#1074#1072#1103' '#1087#1072#1087#1082#1072
    FileTypes = <>
    Options = []
    Left = 160
    Top = 32
  end
end
