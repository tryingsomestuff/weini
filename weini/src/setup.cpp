#include "analyse.h"
#include "book.h"
#include "cliparser.h"
#include "stats.h"
#include "search.h"
#include "timeman.h"
#include "ttable.h"
#include "thread.h"
#include "util.h"

#include <future>

void AdapteTTSize(){
   if ( Definitions::ttConfig.allTtSize == 0 ){ // manual mode

   }
   else{ // auto mode
       LOG(logINFO) << "As allTtSize key is given (not 0), adapting all tt size to fit in " << Definitions::ttConfig.allTtSize << "Mb (respecting given ratios)";
       unsigned long long sum = Definitions::ttConfig.ttSize
                              + Definitions::ttConfig.ttQSize
                              + Definitions::ttConfig.ttESize
                              + Definitions::ttConfig.ttELSize
                              + Definitions::ttConfig.ttEPSize;
       double factor = (double)Definitions::ttConfig.allTtSize/sum;
       Definitions::ttConfig.ttSize   = (unsigned long long)(factor*Definitions::ttConfig.ttSize);
       Definitions::ttConfig.ttQSize  = (unsigned long long)(factor*Definitions::ttConfig.ttQSize);
       Definitions::ttConfig.ttESize  = (unsigned long long)(factor*Definitions::ttConfig.ttESize);
       Definitions::ttConfig.ttELSize = (unsigned long long)(factor*Definitions::ttConfig.ttELSize);
       Definitions::ttConfig.ttEPSize = (unsigned long long)(factor*Definitions::ttConfig.ttEPSize);
   }
}

bool Setup(int argc, char ** argv){
    srand( (int) time( NULL ) );

    Definitions::InitKeys();

    if (!Definitions::ReadConfig("config.json")) {
        LOG(logWARNING) << "Cannot load local configuration, using the default one";
    }

    CLIParser::Instance().Parse(argc,argv);

    Util::Zobrist::Init();

    Stats::Init();

    Square::InitOffBoard();

    Analyse::InitConstants();

    LOG(logINFO) << "Move::HashType            " << sizeof(Move::HashType);
    LOG(logINFO) << "Zobrist::HashType         " << sizeof(Util::Zobrist::HashType);
    LOG(logINFO) << "Type                      " << sizeof(Piece::eType);
    LOG(logINFO) << "BitBoard                  " << sizeof(BitBoard);
    LOG(logINFO) << "BitBoards                 " << sizeof(BitBoards);
    LOG(logINFO) << "AdditionalPositionInfo    " << sizeof(Game::AdditionalPositionInfo);
    LOG(logINFO) << "Piece                     " << sizeof(Piece);
    LOG(logINFO) << "Player                    " << sizeof(Player);
    LOG(logINFO) << "Move                      " << sizeof(Move);
    LOG(logINFO) << "Position                  " << sizeof(Position) << " (includes BitBoards)";
    LOG(logINFO) << "Square                    " << sizeof(Square);
    LOG(logINFO) << "Transposition             " << sizeof(Transposition);
    LOG(logINFO) << "Bucket                    " << sizeof(Bucket);

    //UtilMove::InitMvvLva(); // not used anymore, there is a table for that

    std::future<bool> bookReadOk = std::async(Book::ReadBook,true);

    TimeMan::Instance().Init();

    AdapteTTSize();

    if (Definitions::ttConfig.do_transpositionTableAlphaBeta
        || Definitions::ttConfig.do_transpositionTableSearch
        || Definitions::ttConfig.do_transpositionTableSortSearch
        || Definitions::ttConfig.do_transpositionTableSortAlphaBeta) {
        Transposition::InitTT();
    }
    if (Definitions::ttConfig.do_transpositionTableQuiesce
        || Definitions::ttConfig.do_transpositionTableQSort) {
        Transposition::InitTTQ();
    }

    if (Definitions::ttConfig.do_transpositionTableEval) {
        Analyse::InitTTE();
    }

    if (Definitions::ttConfig.do_transpositionTableEval) { ///@todo do not depend on do_lazy ???
        Analyse::InitTTL();
    }

    if (Definitions::ttConfig.do_transpositionTableEvalPawn) {
        Analyse::InitTTP();
    }

    ThreadPool<Searcher>::Instance().Setup((int)Definitions::smpConfig.threads);

    LOG(logINFO) << "Waiting for end of book read...";
    bookReadOk.get();
    LOG(logINFO) << "... OK";

    return true;
}
