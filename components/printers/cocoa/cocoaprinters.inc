{%MainUnit ../osprinters.pas}
{**************************************************************
Implementation for carbonprinter
***************************************************************}
uses InterfaceBase, LCLIntf, LCLProc;

const
  CleanPMRect: PMRect = (top: 0; left: 0; bottom: 0; right: 0);
  CleanPMOrientation: PMOrientation = 0;

{ TCocoaPrinterContext }

{function TCocoaPrinterContext.GetSize: TPoint;
var
  R: PMRect;
begin
  Result.X := 0;
  Result.Y := 0;

  if Printer = nil then Exit;
  R:=CleanPMRect;
  if OSError(PMGetAdjustedPaperRect((Printer as TCocoaPrinter).PageFormat, R),
    Self, 'GetSize', 'PMGetUnadjustedPaperRect') then Exit;
    
  Result.X := Round(R.right - R.left);
  Result.Y := Round(R.bottom - R.top);
end;

procedure TCocoaPrinterContext.Release;
begin
  // redirect drawing to dummy context when not able printing page
  CGContext := DefaultContext.CGContext;
end;

procedure TCocoaPrinterContext.Reset;
begin
  inherited Reset;
  
  if CGContext <> nil then
  begin
    // flip and offset CTM from lower to upper left corner
    CGContextTranslateCTM(CGContext, 0, GetSize.Y);
    CGContextScaleCTM(CGContext, 1, -1);
  end;
end;    }

{ TCocoaPrinter }

procedure TCocoaPrinter.CreatePrintSettings;
const
  SName = 'CreatePrintSettings';
begin
  //if OSError(PMCreatePrintSettings(FPrintSettings), Self, SName, 'PMCreatePrintSettings') then
    //raise EPrinter.Create('Error initializing printing for Carbon: Unable to create print settings!');

  //OSError(PMSessionDefaultPrintSettings(PrintSession, FPrintSettings), Self, SName, 'PMSessionDefaultPrintSettings');
end;

function TCocoaPrinter.CreatePageFormat(APaper: String): PMPageFormat;
var
  I: Integer;
  S: TStringList;
const
  SName = 'CreatePageFormat';
begin
{  if APaper = '' then
  begin
    I := -1;
    S := nil;
  end
  else
  begin
    S := TStringList.Create;
    BeginEnumPapers(S);
    I := S.IndexOf(APaper);
  end;
    
  try
    if I < 0 then
    begin
      Result:=nil;
      if OSError(PMCreatePageFormat(Result), Self, SName, 'PMCreatePageFormat') then
        raise EPrinter.Create('Error initializing printing for Carbon: Unable to create page format!');

      OSError(PMSessionDefaultPageFormat(PrintSession, Result), Self, SName, 'PMSessionDefaultPageFormat');
    end
    else
    begin
      OSError(PMCreatePageFormatWithPMPaper(Result,
          PMPaper(CFArrayGetValueAtIndex(FPaperArray, I))),
        Self, SName, 'PMCreatePageFormatWithPMPaper');

    end;
  finally
    if S <> nil then
    begin
      EndEnumPapers;
      S.Free;
    end;
  end;  }
end;

function TCocoaPrinter.ValidatePageFormat: Boolean;
begin
  Result := False;
  {OSError(PMSessionValidatePageFormat(PrintSession, PageFormat, @Result),
    Self, 'ValidatePageFormat', 'PMSessionValidatePageFormat');}
end;

function TCocoaPrinter.ValidatePrintSettings: Boolean;
begin
  Result := False;
  //OSError(PMSessionValidatePrintSettings(PrintSession, PrintSettings, @Result),
    //Self, 'ValidatePrintSettings', 'PMSessionValidatePrintSettings');
end;

function TCocoaPrinter.GetCurrentPrinterName: String;
{var
  P: PMPrinter;    }
begin
 { Result := '';
  P := GetCurrentPrinter;
  if P <> nil then
    Result := CFStringToStr(PMPrinterGetName(P));
  if Trim(Result) = '' then
    Result := '';  }
end;

procedure TCocoaPrinter.BeginPage;
{var
  PaperRect: PMRect;  }
begin
 { if FBeginDocumentStatus = noErr then
  begin
    FNewPageStatus := PMSessionBeginPage(PrintSession, nil, nil);
    OSError(FNewPageStatus, Self, 'BeginPage', 'PMSessionBeginPage', '', kPMCancel);
    
    // update printer context
    if OSError(PMSessionGetCGGraphicsContext(PrintSession, FPrinterContext.CGContext),
      Self, 'BeginPage', 'PMSessionGetCGGraphicsContext') then
        FPrinterContext.Release
      else
        FPrinterContext.Reset;

    // translate the context from his paper (0,0) origin
    // to our working imageable area
    if PMGetAdjustedPaperRect(PageFormat, PaperRect{%H-})=noErr then
      CGContextTranslateCTM(FPrinterContext.CGContext, -PaperRect.left, -PaperRect.top);

    if Assigned(Canvas) then
      Canvas.Handle := HDC(FPrinterContext);
  end;   }
end;

procedure TCocoaPrinter.EndPage;
begin
  {FPrinterContext.Release;
  if Assigned(Canvas) then Canvas.Handle := 0;
  
  if FBeginDocumentStatus = noErr then
  begin
    if FNewPageStatus = noErr then
      OSError(PMSessionEndPage(PrintSession), Self, 'EndPage', 'PMSessionEndPage', '', kPMCancel);
  end;   }
end;

procedure TCocoaPrinter.FindDefaultPrinter;
{var
  P: PMPrinter;
  I, C: CFIndex;
  pa: CFArrayRef;  }
begin
  {pa:=nil;
  if OSError(PMServerCreatePrinterList(kPMServerLocal, pa),
    Self, 'DoEnumPrinters', 'PMServerCreatePrinterList') then Exit;

  if not Assigned(pa) then Exit;

  C := CFArrayGetCount(pa);
  for I := 0 to C - 1 do
  begin
    P := CFArrayGetValueAtIndex(pa, I);

    if PMPrinterIsDefault(P) then
    begin
      FDefaultPrinter := CFStringToStr(PMPrinterGetName(P));
      Break;
    end;
  end;
  CFRelease(pa);    }
end;

constructor TCocoaPrinter.Create;
begin
  inherited Create;

  if PMCreateSession(FPrintSession) <> noErr then
    raise EPrinter.Create('Error initializing printing for Cocoa: Unable to create Carbon print session!');

  CreatePrintSettings;
  FPageFormat := CreatePageFormat('');
  FPrinterContext := TCocoaPrinterContext.Create;
  
  FindDefaultPrinter;
  UpdatePrinter;
  //DebugLn('Current ' + GetCurrentPrinterName);
  //DebugLn('Default ' + FDefaultPrinter);
end;

procedure TCocoaPrinter.DoDestroy;
begin
  FPrinterContext.Free;
  
  {if FPrintSettings <> nil then PMRelease(PMObject(FPrintSettings));
  if FPageFormat <> nil then PMRelease(PMObject(FPageFormat));}
  if FPrintSession <> nil then PMRelease(PMObject(FPrintSession));
  
  inherited DoDestroy;
end;

function TCocoaPrinter.Write(const Buffer; Count: Integer;
  var Written: Integer): Boolean;
begin
  Result := False;
  CheckRawMode(True);

  DebugLn('TCocoaPrinter.Write Error: Raw mode is not supported for Carbon!');
end;

procedure TCocoaPrinter.RawModeChanging;
begin
  //
end;

procedure TCocoaPrinter.Validate;
var
  P: String;
begin
  {ValidatePrintSettings;
  ValidatePageFormat;
  
  // if target paper is not supported, use the default
  P := DoGetPaperName;
  if PaperSize.SupportedPapers.IndexOf(P) = -1 then
    DoSetPaperName(DoGetDefaultPaperName);  }
end;

procedure TCocoaPrinter.UpdatePrinter;
{var
  s: string;
  Res: PMResolution;}
begin
  {s := GetCurrentPrinterName;
  if trim(s) = '' then // Observed if Default printer set to "Use last printer", and no printing done
    s := '*';     // so select lcl default
  SetPrinter(s);
  // set the page format resolution
  Res := GetOutputResolution;
  PMSetResolution(PageFormat, Res);
  Validate; }
end;

function TCocoaPrinter.GetOutputResolution: PMResolution;
var
  res: OSStatus;
begin
{  res := PMPrinterGetOutputResolution(GetCurrentPrinter, PrintSettings, Result{%H-});
  if res<>noErr then
  begin
    Result.vRes:=72;
    Result.hRes:=72;
  end;    }
end;

function TCocoaPrinter.GetXDPI: Integer;
var
  dpi: PMResolution;
begin
  dpi := GetOutputResolution;
  result := round(dpi.hRes);
end;

function TCocoaPrinter.GetYDPI: Integer;
var
  dpi: PMResolution;
begin
  dpi := GetOutputResolution;
  result := round(dpi.hRes);
end;

procedure TCocoaPrinter.DoBeginDoc;
begin
  inherited DoBeginDoc;
  
  //DebugLn('TCocoaPrinter.DoBeginDoc ' + DbgS(Printing));
  Validate;

  //FBeginDocumentStatus := PMSessionBeginCGDocument(PrintSession, PrintSettings, PageFormat);
  //OSError(FBeginDocumentStatus, Self, 'DoBeginDoc', 'PMSessionBeginCGDocument', '', kPMCancel);
    
  FNewPageStatus := kPMCancel;
  
  BeginPage;
end;

procedure TCocoaPrinter.DoNewPage;
begin
  inherited DoNewPage;

  EndPage;
  BeginPage;
end;

procedure TCocoaPrinter.DoEndDoc(aAborded: Boolean);
begin
  inherited DoEndDoc(aAborded);

{  EndPage;
  if FBeginDocumentStatus = noErr then
    OSError(PMSessionEndDocument(PrintSession), Self, 'DoEndDoc', 'PMSessionEndDocument', '', kPMCancel);}
end;

procedure TCocoaPrinter.DoAbort;
begin
  inherited DoAbort;

  //OSError(PMSessionSetError(PrintSession, kPMCancel), Self, 'DoAbort', 'PMSessionSetError');
end;

//Enum all defined printers. First printer it's default
procedure TCocoaPrinter.DoEnumPrinters(Lst: TStrings);
var
  I, PrinterCount: CFIndex;
  NewPrinterNSName: NSString;
  NewPrinterName: String;
  FPrinterArray: NSArray;
  //NewPrinter: NSPrinter;
begin
  FPrinterArray := NSPrinter.printerNames();
  FPrinterArray.retain;
  if FPrinterArray = nil then Exit;

  for I := 0 to FPrinterArray.count - 1 do
  begin
    NewPrinterNSName := NSString(FPrinterArray.objectAtIndex(i));
    NewPrinterName := NSStringToString(NewPrinterNSName);

    // Felipe: This could be used for appending the printer to the TStrings, but what for?
    // also it would be hard to release the NSPrinter later
    //NewPrinter := NSPrinter.printerWithName(NewPrinterNSName);
    //NewPrinter.retain;

    //DebugLn(DbgS(I) + ' ' + PrinterName);
    if NewPrinterName = FDefaultPrinter then
      Lst.InsertObject(0, NewPrinterName, nil{TObject(NewPrinter)})
    else
      Lst.AddObject(NewPrinterName, nil{TObject(NewPrinter)});
  end;
end;

procedure TCocoaPrinter.DoResetPrintersList;
begin
  inherited DoResetPrintersList;
end;

// Cocoa doesn't support this =( We need to use Carbon here
// http://lists.apple.com/archives/cocoa-dev/2005/Nov/msg01227.html
procedure TCocoaPrinter.DoEnumPapers(Lst: TStrings);
var
  P: PMPaper;
  FPaperArray: CFArrayRef;
  I, C: CFIndex;
  CFString: CFStringRef;
  PaperName: String;
  CarbonCurrentPrinter: PMPrinter;
begin
  FPaperArray := nil;

  if PMSessionGetCurrentPrinter(FPrintSession, CarbonCurrentPrinter) <> noErr then Exit;

  if PMPrinterGetPaperList(CarbonCurrentPrinter, FPaperArray) <> noErr then Exit;
  FPaperArray := CFRetain(FPaperArray);

  C := CFArrayGetCount(FPaperArray);
  for I := 0 to C - 1 do
  begin
    P := CFArrayGetValueAtIndex(FPaperArray, I);
    CFString:=nil;
    if PMPaperGetName(P, CFString) <> noErr then Continue;
    PaperName := CFStringToStr(CFString);
    Lst.Add(PaperName);
  end;

  if FPaperArray<>nil then
    CFRelease(FPaperArray);
end;

function TCocoaPrinter.DoGetPaperName: string;
{var
  P: PMPaper;
  CFString: CFStringRef;
const
  SName = 'DoGetPaperName';   }
begin
  {Result := '';

  P:=nil;
  if OSError(PMGetPageFormatPaper(PageFormat, P), Self, SName, 'PMGetPageFormatPaper') then Exit;
  CFString:=nil;
  if OSError(PMPaperGetName(P, CFString), Self, SName, 'PMPaperGetName') then Exit;
  
  Result := CFStringToStr(CFString); }
end;

function TCocoaPrinter.DoGetDefaultPaperName: string;
{var
  T: PMPageFormat;   }
begin
  {Result := '';
  
  T := FPageFormat;
  FPageFormat := CreatePageFormat('');
  
  Result := DoGetPaperName;
  if T <> nil then
  begin
    PMRelease(PMObject(FPageFormat));
    FPageFormat := T;
  end;          }
end;

procedure TCocoaPrinter.DoSetPaperName(AName: string);
var
  O: TPrinterOrientation;
begin
 { O := DoGetOrientation;
  if FPageFormat <> nil then PMRelease(PMObject(FPageFormat));
  
  FPageFormat := CreatePageFormat(AName);
  DoSetOrientation(O);
  
  ValidatePageFormat;   }
end;

function TCocoaPrinter.DoGetPaperRect(AName: string; var APaperRc: TPaperRect): Integer;
{var
  T: PMPageFormat;
  PaperRect, PageRect: PMRect;
  S: Double;
  O: PMOrientation;
  Res: PMResolution;
const
  SName = 'DoGetPaperRect';    }
begin
  {Result := -1;
  
  T := CreatePageFormat(AName);
  try
    // copy scale
    S:=0.0;
    OSError(PMGetScale(PageFormat, S), Self, SName, 'PMGetScale');
    OSError(PMSetScale(T, S), Self, SName, 'PMSetScale');
    
    // copy orientation
    O:=CleanPMOrientation;
    OSError(PMGetOrientation(PageFormat, O), Self, SName, 'PMGetOrientation');
    OSError(PMSetOrientation(T, O, False), Self, SName, 'PMSetOrientation');

    // copy resolution
    Res := GetOutputResolution;
    OSError(PMSetResolution(T, Res), self, SName, 'PMSetResolution');
    
    // update
    OSError(PMSessionValidatePageFormat(PrintSession, T, nil),
      Self, SName, 'PMSessionValidatePageFormat');

    PaperRect:=CleanPMRect;
    OSError(PMGetAdjustedPaperRect(T, PaperRect), Self, SName, 'PMGetAdjustedPaperRect');
    PageRect:=CleanPMRect;
    OSError(PMGetAdjustedPageRect(T, PageRect),  Self, SName, 'PMGetAdjustedPageRect');
  finally
    PMRelease(PMObject(T));
  end;
  
  ValidatePageFormat;
  
  APaperRc.PhysicalRect.Left := 0;
  APaperRc.PhysicalRect.Top := 0;
  APaperRc.PhysicalRect.Right := Round(PaperRect.right - PaperRect.left);
  APaperRc.PhysicalRect.Bottom := Round(PaperRect.bottom - PaperRect.top);
  
  APaperRc.WorkRect.Left := Round(-PaperRect.left);
  APaperRc.WorkRect.Top := Round(-PaperRect.top);
  APaperRc.WorkRect.Right := Round(PageRect.right - PageRect.left - PaperRect.left);
  APaperRc.WorkRect.Bottom := Round(PageRect.bottom - PageRect.top - PaperRect.top);
  
  Result := 1;  }
end;

function TCocoaPrinter.DoSetPrinter(aName: string): Integer;
{var
  S: TStringList;
  P: PMPrinter; }
begin
  {S := TStringList.Create;
  BeginEnumPrinters(S);
  try
    Result := S.IndexOf(AName);
    if Result >= 0 then
    begin
      //DebugLn('DoSetPrinter ' + DbgS(Result));
      //DebugLn('TCocoaPrinter.DoSetPrinter ' + AName + ' ' + DbgS(PrintSession) + ' ' + DbgS(Printers.Objects[Result]));
      P := PMPrinter(CFArrayGetValueAtIndex(FPrinterArray, Integer(S.Objects[Result])));
      PMRetain(PMObject(P));
      if OSError(PMSessionSetCurrentPMPrinter(PrintSession, P),
        Self, 'DoSetPrinter', 'PMSessionSetCurrentPMPrinter') then
          raise EPrinter.CreateFmt('The system is unable to select printer "%s"!', [AName]);
    end;
  finally
    EndEnumPrinters;
    S.Free;
  end;   }
end;

function TCocoaPrinter.DoGetCopies: Integer;
var
  C: UInt32;
begin
  {Result := inherited DoGetCopies;
  C:=0;
  if OSError(PMGetCopies(PrintSettings, C), Self, 'DoGetCopies', 'PMGetCopies') then Exit;
  Result := C;  }
end;

procedure TCocoaPrinter.DoSetCopies(AValue: Integer);
begin
  {inherited DoSetCopies(AValue);
  OSError(PMSetCopies(PrintSettings, AValue, False), Self, 'DoSetCopies', 'PMSetCopies');
  
  ValidatePrintSettings;}
end;

function TCocoaPrinter.DoGetOrientation: TPrinterOrientation;
var
  O: PMOrientation;
begin
  Result := inherited DoGetOrientation;
  {O:=CleanPMOrientation;
  if PMGetOrientation(PageFormat, O) <> noErr then Exit;
  
  case O of
    kPMPortrait: Result := poPortrait;
    kPMLandscape: Result := poLandscape;
    kPMReversePortrait: Result := poReversePortrait;
    kPMReverseLandscape: Result := poReverseLandscape;
  end;    }
end;

procedure TCocoaPrinter.DoSetOrientation(AValue: TPrinterOrientation);
var
  O: PMOrientation;
begin
  inherited DoSetOrientation(aValue);

  {case AValue of
    poPortrait: O := kPMPortrait;
    poLandscape: O := kPMLandscape;
    poReversePortrait: O := kPMReversePortrait;
    poReverseLandscape: O := kPMReverseLandscape;
  end;
  
  PMSetOrientation(PageFormat, O, kPMUnlocked);
  ValidatePageFormat;     }
end;

function TCocoaPrinter.GetPrinterType: TPrinterType;
var
  IsRemote: Boolean;
begin
{  Result := ptLocal;
  IsRemote:=false;
  OSError(PMPrinterIsRemote(GetCurrentPrinter,IsRemote), Self, 'GetPrinterType', 'PMPrinterIsRemote');
  if IsRemote then Result := ptNetwork}
end;


function TCocoaPrinter.DoGetPrinterState: TPrinterState;
{var
  State: PMPrinterState;}
begin
  {Result := psNoDefine;

  State:=0;
  if OSError(PMPrinterGetState(GetCurrentPrinter, State), Self, 'DoGetPrinterState', 'PMPrinterGetState') then Exit;
  
  case State of
    kPMPrinterIdle: Result := psReady;
    kPMPrinterProcessing: Result := psPrinting;
    kPMPrinterStopped: Result := psStopped;
  end;}
end;

function TCocoaPrinter.GetCanPrint: Boolean;
begin
  Result := (DoGetPrinterState <> psStopped);
end;

function TCocoaPrinter.GetCanRenderCopies: Boolean;
begin
  Result := True;
end;

initialization

  Printer := TCocoaPrinter.Create;
  
finalization

  FreeAndNil(Printer);