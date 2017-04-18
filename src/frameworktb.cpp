#include "frameworktb.h"


//constructor
FrameworkTB::FrameworkTB (int nArgs, char** argv) {
  //get input arguments
  _inputArgs = new InputArgs();
  _inputArgs->parseArgs(&nArgs,argv);

  //run the TCT controller
  if (nArgs > 1)
  {
    switchCommands();
  }
  else
    throw "FrameworkTB::FrameworkTB : Not enough arguments.";
}



FrameworkTB::~FrameworkTB()
{
}



/*
This routine is a switch between commands.
*/
int32_t FrameworkTB::switchCommands()
{
  if (!_inputArgs->getCommand().compare("convertbintoroot"))
  {
    _cfgParser = new ConfigParser(_inputArgs->getConfig().c_str() );
    convertBinToRoot();
  }
  else if (!_inputArgs->getCommand().compare("convertbintotext"))
  {
    convertBinToText();
  }
  else if (!_inputArgs->getCommand().compare("converttexttoroot"))
  {
    convertTextToRoot();
  }
  else if (!_inputArgs->getCommand().compare("convertrcetoroot"))
  {
    _cfgParser = new ConfigParser(_inputArgs->getConfig().c_str() );
    convertRceToRoot();
  }
  else if (!_inputArgs->getCommand().compare("convertdrstoroot"))
  {
    _cfgParser = new ConfigParser(_inputArgs->getConfig().c_str() );
    convertDrsToRoot();
  }
  else if (!_inputArgs->getCommand().compare("convertdrs3Tcelltoroot"))
  {
    _cfgParser = new ConfigParser(_inputArgs->getConfig().c_str() );
    convertDrs3TcellToRoot();
  }
  else if (!_inputArgs->getCommand().compare("convertlecroy3Tcelltoroot"))
  {
    _cfgParser = new ConfigParser(_inputArgs->getConfig().c_str() );
    convertLeCroy3TcellToRoot();
  }
  else if (!_inputArgs->getCommand().compare("convertdrstotext"))
  {
    _cfgParser = new ConfigParser(_inputArgs->getConfig().c_str() );
    convertDrsToText();
  }
  else if (!_inputArgs->getCommand().compare("check"))
  {
    check();
  }
  else
  {
    throw "Wrong command. Exiting.";
  }
  
  return 0;
}














/*
This routine checks the folders that are entered in the configuration file.
OBSOLETE!!!
*/
int32_t FrameworkTB::check()
{
  cout<<" Checking consistencty of the config file and directories..."<<endl;

  _fileH = new FileHandler();
  _fileH->retrieveRawDataFolderContents();

  _fileH->convDataFolderExists();

  cout<<" Check completed. "<<endl;
  delete _fileH;
  return 0;
}












/* ==========================================================================
 CONVERT LIST TEXT FILE TO JUDITH ROOT
 This routine converts a text file with listed events into
 one Judith format ROOT file.
 ========================================================================== */

int32_t FrameworkTB::convertTextToRoot()
{
  cout<<" Starting conversion of a text file - list - to ROOT format..."<<endl;
  
  //check all folders and read in all data files
  _fileH = new FileHandler();
  
  
  //------------ input/output path handling ---------------------------
  
  //------------set the input txt list file name--------------
  string listSuffix = ".txt";
  string listFile = "";
  
  //if the output is not an empty string
  if ( _inputArgs->getInput().compare("") )
  {
    //check if it ends with ".dat"
    if (!_inputArgs->getInput().compare (
         _inputArgs->getInput().length() - listSuffix.length(),
         listSuffix.length(), listSuffix)  )
    {
      //set the rootFile
      listFile = _inputArgs->getInput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setRawDataFolder( listFile.substr( 0 , listFile.find_last_of("/") )  );
    }
  }
  cout<<" Input list .txt file: "<<listFile<<endl;
  
  
  
  
  //------------set the output root file name--------------
  string rootSuffix = ".root";
  string rootFile = "";
  
  //if the output is not an empty string
  if ( _inputArgs->getOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getOutput().compare (
            _inputArgs->getOutput().length() - rootSuffix.length(),
            rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      rootFile = _inputArgs->getOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setConvDataFolder( rootFile.substr( 0 , rootFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
      rootFile = _fileH->getConvDataFolder() + "outList" + rootSuffix; //set root file name
    }
  }
  cout<<" Output ROOT file: "<<rootFile<<endl;
  
  
  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();
  
  
  
  //------------ end of input/output path handling ---------------------------
  
  
  
  
  
  //start the text converter and open the file and read in everything
  Converters::TextConvert *txtconv = new Converters::TextConvert(listFile);
  
  
  
  
  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents. BUT check that it's not bigger!
  int32_t maxEvents = txtconv->getListNumEvents();
  if ( _inputArgs->getNumEvents() &&
      _inputArgs->getNumEvents() <= maxEvents )
    maxEvents = _inputArgs->getNumEvents();
  
  
  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();
  
  
  //start Judith StorageIO class
  Storage::StorageIO* storage = 0;
  unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
  storage = new Storage::StorageIO(rootFile.c_str(), Storage::OUTPUT, 1, //1 output
                                   treeMask);
  
  
  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting text list into judithROOT file...");
  
  //cout << "max num events"<< maxEvents <<endl;
  
  //---------------Run over all the events in file -----------------------------
  for (int32_t i = 0; i < maxEvents ; i++)
  {
    //S.F debbuging:cout<<" i = "<<i<<" listEvtNo(i)="<<txtconv->getListEvtNo(i)<<endl;
    //verify event number consistency
    if (i != txtconv->getListEvtNo(i) )
      throw "Event number doesn't correspond to the loop number.";
    
    
    //check if we need to skip this event.
    if ( i < skipEvents ) continue;
    
    
    //save to storage
    Storage::Event* storageEvent = 0;
    storageEvent = new Storage::Event( 1 ); //event with one plane
    
    
    
    if ( txtconv->getListIsHit(i) /*if there was a hit according to analyser*/)
    {
      
      Storage::Hit* hit = storageEvent->newHit( 0 ); //hit in plane 1
      hit->setPix(0, 0); //pad detector only has one pixel
      hit->setValue( txtconv->getListCharge(i) ); //charge
      hit->setTiming( txtconv->getListRisetime(i) );
    }
    storageEvent->setTimeStamp( txtconv->getListTimestamp(i) );
    storageEvent->setFrameNumber( txtconv->getListEvtNo(i) );
    storageEvent->setTriggerOffset(0);
    storageEvent->setTriggerInfo( txtconv->getListChi(i) * txtconv->getListChi(i)); //chi
    storageEvent->setInvalid(false);
    
    storage->writeEvent(storageEvent);
    if (storageEvent) delete storageEvent;
    
    
    //update progress bar
    if ( !_inputArgs->getNoBar() ) pb->show(i,maxEvents );
    
    
  }
  //-----------------------end of event loop --------------------------------
  
  if (storage) delete storage;
  delete pb;
  delete txtconv;
  
  return 0;
}




















/* ==========================================================================
                CONVERT RCE BINARY FILE TO JUDITH ROOT
This routine converts a single RCE binary file .dat into a Judith ROOT file.
It needs some input data from the config files: number of planes and mapping
between RCE numbers, outlink numbers and plane numbers.
   ========================================================================== */

int32_t FrameworkTB::convertRceToRoot()
{

  cout<<" Starting conversion of a RCE binary file to ROOT format..."<<endl;

  //check all folders and read in all data files
  _fileH = new FileHandler();



  //------------set the input rce dat file name--------------
  string rceSuffix = ".dat";
  string rceFile = "";

  //if the output is not an empty string
  if ( _inputArgs->getInput().compare("") )
  {
    //check if it ends with ".dat"
    if (!_inputArgs->getInput().compare (
              _inputArgs->getInput().length() - rceSuffix.length(),
              rceSuffix.length(), rceSuffix)  )
    {
      //set the rootFile
      rceFile = _inputArgs->getInput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setRawDataFolder( rceFile.substr( 0 , rceFile.find_last_of("/") )  );
    }
  }
  cout<<" Input RCE .dat file: "<<rceFile<<endl;




  //------------set the output root file name--------------
  string rootSuffix = ".root";
  string rootFile = "";

  //if the output is not an empty string
  if ( _inputArgs->getOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getOutput().compare (
              _inputArgs->getOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      rootFile = _inputArgs->getOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setConvDataFolder( rootFile.substr( 0 , rootFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
      rootFile = _fileH->getConvDataFolder() + "out-judith" + rootSuffix; //set root file name
    }
  }
  cout<<" Output ROOT file: "<<rootFile<<endl;


  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();

  // ------------ end of input and output check ------------------


  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = 0;
  if (_inputArgs->getNumEvents() )
    maxEvents = _inputArgs->getNumEvents();

  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();


  //=====================set up the converter ===========================
  //start the RCE converter. open a binary file.
  Converters::RceConvert *rceconv = new Converters::RceConvert(rceFile);
  std::cout << "config planes.." << std::endl;

  //set the planes
  for (int32_t i = 0; i < _cfgParser->getParVal("Setup","number of planes"); i++)
  {
    stringstream ss;
    ss << "Plane " << i+1;
    string plane = ss.str();
    //cout<<" "<<plane<<endl;
    //cout << "config number: " << _cfgParser->getParVal(plane, "number") << std::endl;
    rceconv->setRcePlane( _cfgParser->getParVal(plane, "isFei4"),
                          _cfgParser->getParStr(plane, "name"),
                          _cfgParser->getParVal(plane, "number"),
                          _cfgParser->getParVal(plane, "rce"),
                          _cfgParser->getParVal(plane, "outlink")
                          );
  }

  //start Judith StorageIO class
  Storage::StorageIO* storage = 0;
  unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
  storage = new Storage::StorageIO( rootFile.c_str(),
                                    Storage::OUTPUT,
                                    _cfgParser->getParVal("Setup","number of planes"), //how many planes
                                    treeMask);


  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting binaries into judithROOT file...");



  //run over events while not end of file -----------------------------------
  //uint64_t old_timestamp = 0;
  while ( rceconv->isGood() )
  {

    //check if we are above the maxEvents (if the value is set above 0)
    if (maxEvents && rceconv->getNumEvents() >= maxEvents) break;


    //read one event from the binary file
    rceconv->readEvent();
    //std::cout << "DONE Read" << std::endl;
    //check if we need to skip this event.
    if ( rceconv->getNumEvents() < skipEvents ) continue;

    //std::cout << "0evt num: " << rceconv->getFileSize() << " " << rceconv->getFileCurrent() << std::endl;
    //cout<<" Timestamp:      "<<(int64_t)rceconv->getEvent()->getTimestamp()<<endl;
    //cout<<" Num. of planes: "<<rceconv->getNumPlanes()<<endl;
    //cout<<" N. of hits:     "<<rceconv->getEvent()->getNumHits()<<endl;

    //Open new event in Root file, with NumPlanes planes
    Storage::Event* storageEvent = 0;
    storageEvent = new Storage::Event( rceconv->getNumPlanes() ); //new event with numPlanes planes
    //std::cout << "planeevt num: " << rceconv->getFileSize() << " " << rceconv->getFileCurrent() << std::endl;

    //std::cout << "hits " <<  rceconv->getEvent()->getNumHits() << std::endl;

    //Write all the hits from rceconv event buffer into judith root event.
    for (int32_t eventHit = 0; eventHit < rceconv->getEvent()->getNumHits(); eventHit++)
    {
      //std::cout << "eventHit: " << eventHit << std::endl;
      //std::cout << "getEvent: " << rceconv->getEvent() << std::endl;
      //std::cout << "getNumHits: " << rceconv->getEvent()->getNumHits() << std::endl;
      //std::cout << "plane: " << rceconv->getEvent()->getHit(eventHit).plane << std::endl;
      //rceconv->getEvent()->getHit(eventHit).print();

      Storage::Hit* hit =
	//storageEvent->newHit( rceconv->getEvent()->getHit(eventHit).plane-1 ); //which plane
	storageEvent->newHit( rceconv->getEvent()->getHit(eventHit).plane ); //which plane.HACK. changed to no subtract 1
      hit->setPix( rceconv->getEvent()->getHit(eventHit).col,
                   rceconv->getEvent()->getHit(eventHit).row ); //pad detector only has one pixel
      //std::cout << "plane: " << rceconv->getEvent()->getHit(eventHit).plane << std::endl;
      hit->setValue( (double)rceconv->getEvent()->getHit(eventHit).value ); //amplitude
      hit->setTiming( (double)rceconv->getEvent()->getHit(eventHit).timing ); //delayof the max amplitude from trigger time
      //std::cout << "saved" << std::endl;
    }
    //std::cout << "done hits: "  << std::endl;
    //timestamp printout
    //uint64_t timestamp = (uint64_t) rceconv->getEvent()->getTimestamp();
    //cout<<rceconv->getNumEvents()<<"\t"<<"TSdiff "<<timestamp-old_timestamp<<endl;
    //old_timestamp = timestamp;
    //std::cout << "evt num: " << rceconv->getFileSize() << " " << rceconv->getFileCurrent() << std::endl;
    //write event specific data into judith event
    storageEvent->setTimeStamp( rceconv->getEvent()->getTimestamp() );
    storageEvent->setFrameNumber( rceconv->getNumEvents() );
    storageEvent->setTriggerOffset(0);
    storageEvent->setTriggerInfo(0);
    storageEvent->setInvalid(false);
    storage->writeEvent(storageEvent);
    if (storageEvent) delete storageEvent;
    //std::cout << "2evt num: " << rceconv->getFileSize() << " " << rceconv->getFileCurrent() << std::endl;
    //check again if end of file.
    if ( !rceconv->isGood() )
    {
      cout<<" End of raw file reached."<<endl;

    }
    //std::cout << "3evt num: " << rceconv->getFileSize() << " " << rceconv->getFileCurrent() << std::endl;    

    //show progress bar
    if ( !_inputArgs->getNoBar() ) pb->show64(rceconv->getFileCurrent(), rceconv->getFileSize() );

    //check file size
    if (rceconv->getFileCurrent() == (rceconv->getFileSize()) )
    {
      cout<<endl<<" End of file reached. "<<endl;
      cout<<" Total number of events written: "<<rceconv->getNumEvents()-skipEvents<<endl;
      break;
    }
  }



  delete rceconv;
  delete pb;
  delete _fileH;
  if (storage) delete storage;
  return 0;
}








/* ==========================================================================
                CONVERT DRS BINARY FILE TO JUDITH ROOT
This routine converts a single DRS binary file .dat into a Judith ROOT file.
It needs some input data from the config files: number of channels and ch mask
   ========================================================================== */

int32_t FrameworkTB::convertDrsToRoot()
{

  cout<<" Starting conversion of a DRS binary file to ROOT format..."<<endl;

  //check all folders and read in all data files
  _fileH = new FileHandler();



  //------------set the input DRS dat file name--------------
  string DRSSuffix = ".dat";
  string DRSFile = "";

  //if the input is not an empty string
  if ( _inputArgs->getInput().compare("") )
  {
    //check if it ends with ".dat"
    if (!_inputArgs->getInput().compare (
              _inputArgs->getInput().length() - DRSSuffix.length(),
              DRSSuffix.length(), DRSSuffix)  )
    {
      //set the rootFile
      DRSFile = _inputArgs->getInput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setRawDataFolder( DRSFile.substr( 0 , DRSFile.find_last_of("/") )  );
    }
  }
  cout<<" Input DRS .dat file: "<<DRSFile<<endl;




  //------------set the output root file name--------------
  string rootSuffix = ".root";
  string rootFile = "";

  //if the output is not an empty string
  if ( _inputArgs->getOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getOutput().compare (
              _inputArgs->getOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      rootFile = _inputArgs->getOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setConvDataFolder( rootFile.substr( 0 , rootFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
      rootFile = _fileH->getConvDataFolder() + "out-judith" + rootSuffix; //set root file name
    }
  }
  cout<<" Output ROOT file: "<<rootFile<<endl;


  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();

  // ------------ end of input and output check ------------------


  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = 0;
  if (_inputArgs->getNumEvents() )
    maxEvents = _inputArgs->getNumEvents();

  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();


  //=====================set up the converter ===========================
  //start the RCE converter. open a binary file.
  Converters::read_DRS *readdrs = new Converters::read_DRS(DRSFile);

  //start the waveform analysis
  WaveformAna *wana = new WaveformAna(
            _cfgParser->getParStr("Waveform analyser", "show pulses"),
            _cfgParser->getParStr("Waveform analyser", "show histograms"),
            _cfgParser->getParFlo("Waveform analyser", "cut max ampl"),
            _cfgParser->getParVal("Waveform analyser", "avg buf len"),
            _cfgParser->getParVal("Waveform analyser", "baseline buf len"),
            "",
            _inputArgs->getTag() //tag = bias voltage and/or sample name
            );

  std::cout << "starting judith output" << std::endl;
  //start Judith StorageIO class
  Storage::StorageIO* storage = 0;
  unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
  storage = new Storage::StorageIO( rootFile.c_str(),
                                    Storage::OUTPUT,
                                    1, //how many planes
                                    treeMask);


  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting DRS binaries into judithROOT file...");


  std::cout << "reading events" << std::endl;
  //run over events while not end of file -----------------------------------
  //uint64_t old_timestamp = 0;
  while ( readdrs->isGood() )
  {
    //Read a DRS event
    readdrs->readEvent();

    //send the waveform to the analyser
    wana->loadWaveform(
      //Number of rows in the array
      (int64_t) 1024,   // (int64_t)
      //Time offset
      0,        //double
      //Time column
      readdrs->getEvent()->_time[0],       //vector of floats
      //Amplitude column
      readdrs->getEvent()->_waveform[0]          //vector of floats
    );


    //update histograms
    wana->updateHistos();

    //write values into root

    //retreive the amplitude and timing.
    //get the timestamp in microseconds

    //cout<<i<<"\t"<<"TSdiff "<<timestamp-old_timestamp<<" \tcorrTSdiff=\t"<<39.0641*(timestamp-old_timestamp)<<endl;
    //old_timestamp = timestamp;


    //save to storage
    Storage::Event* storageEvent = 0;
    storageEvent = new Storage::Event( 1 ); //event with one plane
    if ( wana->getMaxAbsAmplitude() > _cfgParser->getParFlo("Waveform analyser","cut min ampl") /*[V]. if there was a hit according to analyser*/)
    {
      Storage::Hit* hit = storageEvent->newHit( 0 ); //hit in plane 1
      hit->setPix(0, 0); //pad detector only has one pixel
      hit->setTiming(2); //delayof the max amplitude from trigger time
      hit->setValue( (double)wana->getMaxAbsAmplitude() ); //amplitude

    }


    storageEvent->setTimeStamp( readdrs->getEvent()->getTimestamp() );
    storageEvent->setFrameNumber( readdrs->getEvent()->_eh.event_serial_number );
    storageEvent->setTriggerOffset(0);
    storageEvent->setTriggerInfo(0);
    storageEvent->setInvalid( wana->isInvalid() );
    storage->writeEvent(storageEvent);
    if (storageEvent) delete storageEvent;


  }


  if (readdrs) delete readdrs;
  if (pb) delete pb;
  if (_fileH) delete _fileH;
  if (storage) delete storage;
  return 0;
}






/* ==========================================================================
                CONVERT DRS BINARY FILE FROM 3T CELL TO JUDITH ROOT
This routine converts a single DRS binary file .dat into a Judith ROOT file.
It needs some input data from the config files: number of channels and ch mask
   ========================================================================== */

int32_t FrameworkTB::convertDrs3TcellToRoot()
{

  cout<<" Starting conversion of a DRS binary file from 3T cell to ROOT format..."<<endl;

  //check all folders and read in all data files
  _fileH = new FileHandler();



  //------------set the input DRS dat file name--------------
  string DRSSuffix = ".dat";
  string DRSFile = "";

  //if the input is not an empty string
  if ( _inputArgs->getInput().compare("") )
  {
    //check if it ends with ".dat"
    if (!_inputArgs->getInput().compare (
              _inputArgs->getInput().length() - DRSSuffix.length(),
              DRSSuffix.length(), DRSSuffix)  )
    {
      //set the rootFile
      DRSFile = _inputArgs->getInput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setRawDataFolder( DRSFile.substr( 0 , DRSFile.find_last_of("/") )  );
    }
  }
  cout<<" Input DRS .dat file: "<<DRSFile<<endl;

  //------------set the output root file name--------------
  string rootSuffix = ".root";
  string rootFile = "";
  string ntupleFileName = "";

  //if the output is not an empty string
  if ( _inputArgs->getOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getOutput().compare (
              _inputArgs->getOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      rootFile = _inputArgs->getOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setConvDataFolder( rootFile.substr( 0 , rootFile.find_last_of("/") )  );
      ntupleFileName = _inputArgs->getOutput();
      ntupleFileName = ntupleFileName.substr(0, ntupleFileName.find(rootSuffix));
      ntupleFileName = ntupleFileName + "-ntuple" + rootSuffix;
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
      rootFile = _fileH->getConvDataFolder() + "out-judith" + rootSuffix; //set root file name
      ntupleFileName = _fileH->getConvDataFolder() + "out-judith-ntuple" + rootSuffix;
    }
  }
  cout<<" Output ROOT file: "<<rootFile<<endl;
  cout<<" Output NTuple file: "<<ntupleFileName<<endl;
  
  //Create the NTuple and its file
  TFile *ntupleFile = new TFile(ntupleFileName.c_str(),"RECREATE");
  //if(ntupleFile->IsOpen()) cout << "Ntuple File Open" << endl;
  TNtuple *ntuple = new TNtuple("ntuple","data from ascii file","event:time:hit:a:m:c:to:b:chi:invalid");

  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();

  // ------------ end of input and output check ------------------


  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = 0;
  if (_inputArgs->getNumEvents() )
    maxEvents = _inputArgs->getNumEvents();

  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();


  //=====================set up the converter ===========================
  //start the RCE converter. open a binary file.
  Converters::read_DRS *readdrs = new Converters::read_DRS(DRSFile);

  //start the waveform analysis
  WaveformAna3Tcell *wana = new WaveformAna3Tcell(
            _cfgParser->getParStr("Waveform analyser", "show pulses"),
            _cfgParser->getParStr("Waveform analyser", "show histograms"),
            _cfgParser->getParVal("Waveform analyser", "avg buf len")
            );
  
  wana->setCuts(_cfgParser->getParValFloat("Waveform analyser", "chi2 cut"),
                _cfgParser->getParValFloat("Waveform analyser", "simple threshold cut"),
                _cfgParser->getParValFloat("Waveform analyser", "time threshold cut"),
                _cfgParser->getParValFloat("Waveform analyser", "time cut"),
                _cfgParser->getParVal("Waveform analyser", "relax factor time"),
                _cfgParser->getParVal("Waveform analyser", "relax factor threshold"),
                _cfgParser->getParValFloat("Waveform analyser", "min hit detection time"),
                _cfgParser->getParValFloat("Waveform analyser", "max hit detection time")
                );


  //start Judith StorageIO class
  Storage::StorageIO* storage = 0;
  unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
  storage = new Storage::StorageIO( rootFile.c_str(),
                                    Storage::OUTPUT,
                                    1, //how many planes
                                    treeMask);


  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting DRS binaries from 3T cell into judithROOT file...");



  //run over events while not end of file -----------------------------------
  //uint64_t old_timestamp = 0;
  int hitCnt = 0;
  while ( readdrs->isGood() )
  {
    //Read a DRS event
    readdrs->readEvent();


    //send the reset waveform to the analyser
    wana->loadResetWaveform(
      //Number of rows in the array
      (int64_t) 1024,   // (int64_t)
      //Time offset
      0,        //double
      //Time column
      readdrs->getEvent()->_time[3],       //vector of floats
      //Amplitude column
      readdrs->getEvent()->_waveform[3]          //vector of floats
    );

    //send the waveform to the analyser
    wana->loadWaveform(
      //Number of rows in the array
      (int64_t) 1024,   // (int64_t)
      //Time offset
      0,        //double
      //Time column
      readdrs->getEvent()->_time[2],       //vector of floats
      //Amplitude column
      readdrs->getEvent()->_waveform[2]          //vector of floats
    );

    // Analyze the 3T cell waveform
    bool hit = false;
    float stepSize = 0;
    float timeConstant = 0;
    float hitDetectionTime = 0;
    float offset = 0;
    float slope = 0;
    float chi2 = 0;
    double resetTime = 0;
    wana->analyze3TCellWaveform(hit, stepSize, timeConstant, hitDetectionTime, offset, slope, chi2,resetTime);

    if(wana->isInvalid())
      cout << "!!!Event invalid!!!" << endl;
    
    if(hit){
      hitCnt++;
      cout << "\tDetected hit number " << hitCnt << " with step size of " << stepSize << " and time constant of " << timeConstant << " and hit detection time of " << hitDetectionTime << " and Chi2 of " << chi2 << endl;
      cout << "---------- End of hit processing ----------\n" << endl;
    }


    ntuple->Fill(readdrs->getEvent()->_eh.event_serial_number,readdrs->getEvent()->getTimestamp(),hit,offset,slope,timeConstant,hitDetectionTime,abs(stepSize),chi2, wana->isInvalid());
    //ntuple->Write();
    //update histograms
    if(hit && (!_cfgParser->getParStr("Waveform analyser", "show histograms").compare("true")))
      wana->updateHistos();

    //write values into root

    //retreive the amplitude and timing.
    //get the timestamp in microseconds

    //cout<<i<<"\t"<<"TSdiff "<<timestamp-old_timestamp<<" \tcorrTSdiff=\t"<<39.0641*(timestamp-old_timestamp)<<endl;
    //old_timestamp = timestamp;


    //save to storage
    Storage::Event* storageEvent = 0;
    storageEvent = new Storage::Event( 1 ); //event with one plane
    if (hit)
    {
      Storage::Hit* hit = storageEvent->newHit( 0 ); //hit in plane 1
      hit->setPix(0, 0); //pad detector only has one pixel
      hit->setTiming(timeConstant); //Charge collection time
      hit->setValue(stepSize); //amplitude
      hit->setT0(hitDetectionTime);
      hit->setChi2(chi2);
    }


    storageEvent->setTimeStamp( readdrs->getEvent()->getTimestamp() );
    storageEvent->setFrameNumber( readdrs->getEvent()->_eh.event_serial_number );
    storageEvent->setTriggerOffset(0);
    storageEvent->setTriggerInfo(0);
    storageEvent->setInvalid(wana->isInvalid()); //To think about when event shall be considered invalid
    storage->writeEvent(storageEvent);
    if (storageEvent) delete storageEvent;
    //if (wana) delete wana;


  }
  
  ntupleFile->Write();
  ntupleFile->Close();

    cout << "\n\n********** FINISHED WAVE FORM ANALYSIS SUCCESSFULLY **********\n\n" << endl;
    cout << "press any key to exit" << endl;


  getchar();



  if (readdrs) delete readdrs;
  if (pb) delete pb;
  if (_fileH) delete _fileH;
  if (storage) delete storage;
  if (wana) delete wana;
  
  return 0;
}







/* ==========================================================================
                CONVERT LeCroy BINARY FILE FROM 3T CELL TO JUDITH ROOT
This routine converts a single DRS binary file .dat into a Judith ROOT file.
It needs some input data from the config files: number of channels and ch mask
   ========================================================================== */

int32_t FrameworkTB::convertLeCroy3TcellToRoot()
{
  
  cout<<" Starting conversion of a LeCroy binary file from 3T cell to ROOT format..."<<endl;

  //check all folders and read in all data files
  _fileH = new FileHandler();

 //------------ input/output path handling ---------------------------

  //suffix for ROOT files.
  string rootSuffix = ".root";
  string rootFile ="";
  string resultsFile ="";

  //----------set input folder ----------------
  //the input folder was set via an argument.
  _fileH->setRawDataFolder( _inputArgs->getInput() ); //set the folder to this.


  //----------set output folder or file ----------------
  //if the output is not an empty string
  if ( _inputArgs->getOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getOutput().compare (
              _inputArgs->getOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      rootFile = _inputArgs->getOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setConvDataFolder( rootFile.substr( 0 , rootFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
      rootFile = _fileH->getConvDataFolder() + "out" + rootSuffix; //set root file name
    }
  }


  //-------------optional! if we want to save the analysis results
  //if the output is not an empty string
  if ( _inputArgs->getResOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getResOutput().compare (
              _inputArgs->getResOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      resultsFile = _inputArgs->getResOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setResDataFolder( resultsFile.substr( 0 , resultsFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setResDataFolder( _inputArgs->getResOutput() ); //set the folder to this.
      resultsFile = _fileH->getResDataFolder() + "outResults" + rootSuffix; //set root file name
    }
    //check the consistency of the folder for saving the results file
    _fileH->resDataFolderExists();
  }

  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();


  //print out file name.
  cout<<" Judith file name:  "<<rootFile<<endl;
  cout<<" Results file name: "<<resultsFile<<endl;


  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = _fileH->getNumRawFiles();
  if (_inputArgs->getNumEvents() &&
      _inputArgs->getNumEvents() <= maxEvents)
    maxEvents = _inputArgs->getNumEvents();

  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();

  //------------ end of input/output path handling ---------------------------

  //start the binary converter
  Converters::LeCroyBin *lcb;

  //start the waveform analysis
  WaveformAna3Tcell *wana = new WaveformAna3Tcell(
            _cfgParser->getParStr("Waveform analyser", "show pulses"),
            _cfgParser->getParStr("Waveform analyser", "show histograms"),
            _cfgParser->getParVal("Waveform analyser", "avg buf len")
            );
  
  wana->setCuts(_cfgParser->getParValFloat("Waveform analyser", "chi2 cut"),
                _cfgParser->getParValFloat("Waveform analyser", "simple threshold cut"),
                _cfgParser->getParValFloat("Waveform analyser", "time threshold cut"),
                _cfgParser->getParValFloat("Waveform analyser", "time cut"),
                _cfgParser->getParVal("Waveform analyser", "relax factor time"),
                _cfgParser->getParVal("Waveform analyser", "relax factor threshold"),
                _cfgParser->getParValFloat("Waveform analyser", "min hit detection time"),
                _cfgParser->getParValFloat("Waveform analyser", "max hit detection time")
                );


  //start Judith StorageIO class
  Storage::StorageIO* storage = 0;
  unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
  storage = new Storage::StorageIO( rootFile.c_str(),
                                    Storage::OUTPUT,
                                    1, //how many planes
                                    treeMask);


  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting DRS binaries from 3T cell into judithROOT file...");



  //run over events while not end of file -----------------------------------
  //uint64_t old_timestamp = 0;
  for (int32_t i = 0; i < maxEvents; i++)
  {
    //open a file with lecroy converter.
    string currentFile = _fileH->getRawDataFolder() + _fileH->getRawFile(i);


    //read a single binary file
    lcb = new Converters::LeCroyBin();
    lcb->readFile(currentFile);

    //send the waveform to the analyser
    wana->loadWaveform(
      //Number of rows in the array
      (int64_t) lcb->WAVE_ARRAY_COUNT,   // (int64_t)
      //Time offset
      lcb->HORIZ_OFFSET,        //double
      //Time column
      lcb->pTimingArray1,       //vector of floats
      //Amplitude column
      lcb->pDataArray1          //vector of floats
    );
    
    //update histograms
    wana->updateHistos();

    // Analyze the 3T cell waveform
    bool hit = false;
    float stepSize = 0;
    float timeConstant = 0;
    float hitDetectionTime = 0;
    float offset = 0;
    float slope = 0;
    float chi2 = 0;
    double resetTime = 0;
    wana->analyze3TCellWaveform(hit, stepSize, timeConstant, hitDetectionTime, offset, slope, chi2,resetTime);

    if(wana->isInvalid())
      cout << "!!!Event invalid!!!" << endl;
    
    if(hit){
      //hitCnt++;
      cout << "\tDetected hit number " << i << " with step size of " << stepSize << " and time constant of " << timeConstant << " and hit detection time of " << hitDetectionTime << " and Chi2 of " << chi2 << endl;
      cout << "---------- End of hit processing ----------\n" << endl;
    }


    //ntuple->Fill(readdrs->getEvent()->_eh.event_serial_number,readdrs->getEvent()->getTimestamp(),hit,offset,slope,timeConstant,hitDetectionTime,abs(stepSize),chi2, wana->isInvalid());
    //ntuple->Write();
    //update histograms
    if(hit && (!_cfgParser->getParStr("Waveform analyser", "show histograms").compare("true")))
      wana->updateHistos();

    //write values into root

    //retreive the amplitude and timing.
    //get the timestamp in microseconds

    //retreive the amplitude and timing.
    //get the timestamp in microseconds
    uint64_t timestamp = (uint64_t)
    ( ( (double) lcb->TRIGGER_TIME_sec +
        (double) lcb->TRIGGER_TIME_min*60 +
        (double) lcb->TRIGGER_TIME_hour*3660 ) * 1e6 );  //Timestamp in microsecs.
    
    //cout<<i<<"\t"<<"TSdiff "<<timestamp-old_timestamp<<" \tcorrTSdiff=\t"<<39.0641*(timestamp-old_timestamp)<<endl;
    //old_timestamp = timestamp;


    //save to storage
    Storage::Event* storageEvent = 0;
    storageEvent = new Storage::Event( 1 ); //event with one plane
    if (hit)
    {
      Storage::Hit* hit = storageEvent->newHit( 0 ); //hit in plane 1
      hit->setPix(0, 0); //pad detector only has one pixel
      hit->setTiming(timeConstant); //Charge collection time
      hit->setValue(stepSize); //amplitude
      hit->setT0(hitDetectionTime);
      hit->setChi2(chi2);
      hit->setSlope(slope);
      hit->setOffset(offset);
    }


    storageEvent->setTimeStamp( timestamp );
    storageEvent->setResetTime( resetTime );
    storageEvent->setFrameNumber( i );
    storageEvent->setTriggerOffset(0);
    storageEvent->setTriggerInfo(0);
    storageEvent->setInvalid( wana->isInvalid() );
    storage->writeEvent(storageEvent);
    if (storageEvent) delete storageEvent;

    //update progress bar
    if ( !_inputArgs->getNoBar() ) pb->show(i,maxEvents );
    delete lcb;
  }

  //save in results file if required by the input args.
  //update histograms
  //wana->updateHistos("SAVEANDCLOSE");
  
  //ntupleFile->Write();
  //ntupleFile->Close();

    cout << "\n\n********** FINISHED WAVE FORM ANALYSIS SUCCESSFULLY **********\n\n" << endl;
    cout << "press any key to exit" << endl;


  getchar();



  if (pb) delete pb;
  if (_fileH) delete _fileH;
  if (storage) delete storage;
  if (wana) delete wana;
  
  return 0;
}







/* ==========================================================================
                CONVERT LECROY BINARY TO JUDITH ROOT
This routine converts a set of LeCroy binary waveform files into
one Judith format ROOT file. It contains a waveform analyser
which finds an amplitude and its time in the signal.
   ========================================================================== */
int32_t FrameworkTB::convertBinToRoot()
{
  cout<<" Starting conversion of binary files to ROOT format..."<<endl;

  //check all folders and read in all data files
  _fileH = new FileHandler();



  //------------ input/output path handling ---------------------------

  //suffix for ROOT files.
  string rootSuffix = ".root";
  string rootFile ="";
  string resultsFile ="";

  //----------set input folder ----------------
  //the input folder was set via an argument.
  _fileH->setRawDataFolder( _inputArgs->getInput() ); //set the folder to this.


  //----------set output folder or file ----------------
  //if the output is not an empty string
  if ( _inputArgs->getOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getOutput().compare (
              _inputArgs->getOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      rootFile = _inputArgs->getOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setConvDataFolder( rootFile.substr( 0 , rootFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
      rootFile = _fileH->getConvDataFolder() + "out" + rootSuffix; //set root file name
    }
  }


  //-------------optional! if we want to save the analysis results
  //if the output is not an empty string
  if ( _inputArgs->getResOutput().compare("") )
  {
    //check if it ends with ".root"
    if (!_inputArgs->getResOutput().compare (
              _inputArgs->getResOutput().length() - rootSuffix.length(),
              rootSuffix.length(), rootSuffix)  )
    {
      //set the rootFile
      resultsFile = _inputArgs->getResOutput();
      //to check that the folder for this file exists, we need to chop down the rootFile path.
      _fileH->setResDataFolder( resultsFile.substr( 0 , resultsFile.find_last_of("/") )  );
    }
    else //otherwise regard it as a FOLDER.
    {
      //if it doesn't end in .root then it's a folder
      _fileH->setResDataFolder( _inputArgs->getResOutput() ); //set the folder to this.
      resultsFile = _fileH->getResDataFolder() + "outResults" + rootSuffix; //set root file name
    }
    //check the consistency of the folder for saving the results file
    _fileH->resDataFolderExists();
  }



  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();


  //print out file name.
  cout<<" Judith file name:  "<<rootFile<<endl;
  cout<<" Results file name: "<<resultsFile<<endl;


  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = _fileH->getNumRawFiles();
  if (_inputArgs->getNumEvents() &&
      _inputArgs->getNumEvents() <= maxEvents)
    maxEvents = _inputArgs->getNumEvents();

  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();

  //------------ end of input/output path handling ---------------------------

  //start the binary converter
  Converters::LeCroyBin *lcb;


  //start the waveform analysis
  WaveformAna *wana = new WaveformAna(
            _cfgParser->getParStr("Waveform analyser", "show pulses"),
            _cfgParser->getParStr("Waveform analyser", "show histograms"),
            _cfgParser->getParFlo("Waveform analyser", "cut max ampl"),
            _cfgParser->getParVal("Waveform analyser", "avg buf len"),
            _cfgParser->getParVal("Waveform analyser", "baseline buf len"),
            resultsFile,
            _inputArgs->getTag() //tag = bias voltage and/or sample name
            );


  //start Judith StorageIO class
  Storage::StorageIO* storage = 0;
  unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
  storage = new Storage::StorageIO(rootFile.c_str(), Storage::OUTPUT, 1, //1 output
                                     treeMask);


  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting binaries into judithROOT file...");


  //uint64_t old_timestamp = 0;
  //---------------Run over all the raw files -------------------------------
  for (int32_t i = 0; i < maxEvents; i++)
  {

    //check if we need to skip this event.
    if ( i < skipEvents ) continue;


    //open a file with lecroy converter.
    string currentFile = _fileH->getRawDataFolder() + _fileH->getRawFile(i);


    //read a single binary file
    lcb = new Converters::LeCroyBin();
    lcb->readFile(currentFile);


    //send the waveform to the analyser
    wana->loadWaveform(
      //Number of rows in the array
      (int64_t) lcb->WAVE_ARRAY_COUNT,   // (int64_t)
      //Time offset
      lcb->HORIZ_OFFSET,        //double
      //Time column
      lcb->pTimingArray1,       //vector of floats
      //Amplitude column
      lcb->pDataArray1          //vector of floats
    );


    //update histograms
    wana->updateHistos();


    //retreive the amplitude and timing.
    //get the timestamp in microseconds
    uint64_t timestamp = (uint64_t)
    ( ( (double) lcb->TRIGGER_TIME_sec +
        (double) lcb->TRIGGER_TIME_min*60 +
        (double) lcb->TRIGGER_TIME_hour*3660 ) * 1e6 );  //Timestamp in microsecs.

    //cout<<i<<"\t"<<"TSdiff "<<timestamp-old_timestamp<<" \tcorrTSdiff=\t"<<39.0641*(timestamp-old_timestamp)<<endl;
    //old_timestamp = timestamp;


    //save to storage
    Storage::Event* storageEvent = 0;
    storageEvent = new Storage::Event( 1 ); //event with one plane
    if ( wana->getMaxAbsAmplitude() > _cfgParser->getParFlo("Waveform analyser","cut min ampl") /*[V]. if there was a hit according to analyser*/)
    {
      Storage::Hit* hit = storageEvent->newHit( 0 ); //hit in plane 1
      hit->setPix(0, 0); //pad detector only has one pixel
      hit->setTiming(2); //delayof the max amplitude from trigger time
      hit->setValue( (double)wana->getMaxAbsAmplitude() ); //amplitude

    }


    storageEvent->setTimeStamp( timestamp );
    storageEvent->setFrameNumber( i );
    storageEvent->setTriggerOffset(0);
    storageEvent->setTriggerInfo(0);
    storageEvent->setInvalid( wana->isInvalid() );
    storage->writeEvent(storageEvent);
    if (storageEvent) delete storageEvent;


    //update progress bar
    if ( !_inputArgs->getNoBar() ) pb->show(i,maxEvents );
    delete lcb;

  }

  //save in results file if required by the input args.
  //update histograms
  wana->updateHistos("SAVEANDCLOSE");

  if (storage) delete storage;
  delete pb;
  delete wana;
  delete _fileH;

  return 0;
}























/* ==========================================================================
                    CONVERT LECROY BINARY TO TEXT WAVEFORM
This routine converts a set of LeCroy binary waveform files into a set
of text format waveform files.
   ========================================================================== */
int32_t FrameworkTB::convertBinToText()
{
  cout<<" Starting conversion of binary files to TXT format..."<<endl;


  //check all folders and read in all data files
  _fileH = new FileHandler();


  // the input folder was input via an argument.
  _fileH->setRawDataFolder( _inputArgs->getInput() ); //set the folder to this.

  // the output folder was input via an argument.
  _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.

  //check for consistency of the input and output folders while opening/creating.
  _fileH->retrieveRawDataFolderContents();
  _fileH->convDataFolderExists();


  //start the binary converter
  Converters::LeCroyBin *lcb;


  //start the text converter;
  Converters::TextConvert *txtconv;


  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = _fileH->getNumRawFiles();
  if (_inputArgs->getNumEvents() &&
      _inputArgs->getNumEvents() <= maxEvents )
    maxEvents = _inputArgs->getNumEvents();

  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();


  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting binaries into text files...");


  //---------------Run over all the raw files -------------------------------
  for (int32_t i = 0; i < maxEvents; i++)
  {

    //check if we need to skip this event.
    if ( i < skipEvents ) continue;


    //open a file with lecroy converter and save it to a text file.
    string currentFile = _fileH->getRawDataFolder() + _fileH->getRawFile(i);
    string textFile = _fileH->getConvDataFolder() + _fileH->getRawFile(i);
    textFile = textFile.erase(textFile.size()-4, textFile.size());
    textFile+= ".txt";


    //read a single binary file
    lcb = new Converters::LeCroyBin();
    lcb->readFile(currentFile);


    //initialize text storage
    txtconv = new Converters::TextConvert();


    //get the timestamp in microseconds
    uint64_t timestamp = (uint64_t)
    ( ( (double) lcb->TRIGGER_TIME_sec +
        (double) lcb->TRIGGER_TIME_min*60 +
        (double) lcb->TRIGGER_TIME_hour*3660 ) * 1e6 );  //Timestamp in microsecs.


    //open a text file with the same name and different extension
    txtconv->openTextDataFile(textFile);


    //write header to the file
    txtconv->writeHeaderToTDF(
      //Evt Number
      i,
      //Timestamp - ULong64_t
      timestamp,
      //Horizontal interval for data points
      lcb->HORIZ_INTERVAL
    );


    //write data to the file
    txtconv->writeDataToTDF(
      //Number of rows in the array
      (int64_t) lcb->WAVE_ARRAY_COUNT,   // (int64_t)
      //Time offset
      lcb->HORIZ_OFFSET,        //double
      //Time column
      lcb->pTimingArray1,       //vector of floats
      //Amplitude column
      lcb->pDataArray1          //vector of floats
    );


    //close the file
    txtconv->closeTextDataFile();


    //update progress bar
    if ( !_inputArgs->getNoBar() ) pb->show(i,maxEvents );
    delete lcb;
    delete txtconv;

  } // ------------------------------------------------------

  delete pb;
  delete _fileH;

  return 0;
}




/* ==========================================================================
 CONVERT DRS TO TEXT WAVEFORM
 This routine converts a set of DRS binary files to a set
 of text format waveform files.
 ========================================================================== */
int32_t FrameworkTB::convertDrsToText()
{

    
    cout<<" Starting conversion of a DRS binary file to TEXT format..."<<endl;
    
    //check all folders and read in all data files
    _fileH = new FileHandler();
    
    
    
    //------------set the input DRS dat file name--------------
    string DRSSuffix = ".dat";
    string DRSFile = "";
    string DRSFileOnly = "";
  
    //if the input is not an empty string
    if ( _inputArgs->getInput().compare("") )
    {
      //check if it ends with ".dat"
      if (!_inputArgs->getInput().compare (
                                           _inputArgs->getInput().length() - DRSSuffix.length(),
                                           DRSSuffix.length(), DRSSuffix)  )
      {
        //set the rootFile
        DRSFile = _inputArgs->getInput();
        //to check that the folder for this file exists, we need to chop down the rootFile path.
        _fileH->setRawDataFolder( DRSFile.substr( 0 , DRSFile.find_last_of("/") )  );
        DRSFileOnly= DRSFile.substr( DRSFile.find_last_of("/")+1, DRSFile.size() );
//        cout<<"DRSFILEONLY "<<DRSFileOnly<<endl;
      }
    }
    cout<<" Input DRS .dat file: "<<DRSFile<<endl;
  
  //=====================set up the converter ===========================
  //start the RCE converter. open a binary file.
  Converters::read_DRS *readdrs = new Converters::read_DRS(DRSFile);
 


  
  //----------set output folder or file ----------------
  //if the output is not an empty string
  string textFile = "";
  if ( _inputArgs->getOutput().compare("") )
  {
    //if it doesn't end in .root then it's a folder
    _fileH->setConvDataFolder( _inputArgs->getOutput() ); //set the folder to this.
    //Text file names
    textFile = _fileH->getConvDataFolder() + DRSFileOnly;
    textFile = textFile.erase(textFile.size()-4, textFile.size());
  }

  
  //start the text converter;
  Converters::TextConvert *txtconv;
  
  
  //check if argument with nr. of events exists. if yes, then use
  // this value for maxEvents.
  int32_t maxEvents = pow(2.0,30);
  if (_inputArgs->getNumEvents() &&
      _inputArgs->getNumEvents() <= maxEvents )
    maxEvents = _inputArgs->getNumEvents();
  
  //check of argument with nr. of SKIPPED events exists. If yes, then
  //use this value for skippedEvents.
  int32_t skipEvents = 0;
  if (_inputArgs->getSkipEvents() )
    skipEvents = _inputArgs->getSkipEvents();
  
  
  //initialize progress bar
  ProgBar *pb = new ProgBar("\n Converting binaries into text files...");
 

  
  //---------------Run over all the raw files -------------------------------
  for (int32_t i = 0; i < maxEvents; i++)
  {
    
    //check if we need to skip this event.
    if ( i < skipEvents ) continue;
    

    //open a file with lecroy converter and save it to a text file.
    stringstream curFileName;
    curFileName << textFile << "_" << setw(7) << setfill('0') << i << setfill(' ')<< ".txt";
    
//    //read a single binary file
//    lcb = new Converters::LeCroyBin();
//    lcb->readFile(currentFile);
    readdrs->readEvent();
    
    
    //initialize text storage
    txtconv = new Converters::TextConvert();
    
    
    //get the timestamp in microseconds
//    uint64_t timestamp = (uint64_t)
//    ( ( (double) lcb->TRIGGER_TIME_sec +
//       (double) lcb->TRIGGER_TIME_min*60 +
//       (double) lcb->TRIGGER_TIME_hour*3660 ) * 1e6 );  //Timestamp in microsecs.
    uint64_t timestamp = readdrs->getEvent()->getTimestamp() ;
    
    
    //open a text file with the same name and different extension
    txtconv->openTextDataFile(curFileName.str() );
    
    
    //write header to the file
    txtconv->writeHeaderToTDF(
                              //Evt Number
                              i,
                              //Timestamp - ULong64_t
                              timestamp,
                              //Horizontal interval for data points - sampling rate
                              0.2
                              );
    
    
    //write data to the file
    txtconv->writeDataToTDF(
                            //Number of rows in the array
                            1024,   // (int64_t)
                            //Time offset
                            0,        //double
                            //Time column
                            readdrs->getEvent()->_time[0],       //vector of floats
                            //Amplitude column
                            readdrs->getEvent()->_waveform[0]          //vector of floats
                            );
    
    
    //close the file
    txtconv->closeTextDataFile();
    
    
    //update progress bar
    if ( !_inputArgs->getNoBar() ) pb->show(i,maxEvents );
    delete txtconv;
    
  } // ------------------------------------------------------
  delete readdrs;
  
  delete pb;
  delete _fileH;
  
  return 0;

    
    

  
  
  
  
}