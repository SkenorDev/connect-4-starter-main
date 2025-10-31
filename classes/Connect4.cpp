#include "Connect4.h"


Connect4::Connect4()
{
    _grid = new Grid(7, 6);
}

Connect4::~Connect4()
{
    delete _grid;
}


Bit* Connect4::PieceForPlayer(int pieceType)
{
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    bool isRed = (pieceType == RED_PIECE);
    bit->LoadTextureFromFile(isRed ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(isRed ? RED_PLAYER : YELLOW_PLAYER));
    return bit;
}

void Connect4::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _gameOptions.AIMAXDepth =3;
    _grid->initializeSquares(80, "square.png");


    startGame();
}

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool Connect4::actionForEmptyHolder(BitHolder &holder)
{
 ImVec2 pos= holder.getPosition();
     //_grid->getHolder(pos.x, pos.y);
     ChessSquare* square = static_cast<ChessSquare*>(&holder);
      int x = square->getColumn();
      int y = square->getRow();
         auto  below = ownerAt(x,y);
      if(y!=6){
      below = ownerAt(x,y+1);
      }
   
    // int x = static_cast<int>(pos.x);
    // int y = static_cast<int>(pos.y);
    // y=y-40;
    // y=y/80;
    // x=x-40;
    // x=x/80;
    // printf("Clicked holder at position: x=%d, y=%d\n", x, y);

     if ( holder.bit()) {
        return false;
    }
if (below == nullptr && y != 5) {
    return false;
}
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber() == 0 ? RED_PLAYER : YELLOW_PLAYER);

  
    if (bit) {
        auto topSquare = _grid->getSquare(x,0);
        bit->setPosition(topSquare->getPosition());
        bit->moveTo(holder.getPosition());
        holder.setBit(bit);
        endTurn();
        
        return true;
    }   
    return false;
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // you can't move anything in tic tac toe
    return false;
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    // you can't move anything in tic tac toe
    return false;
}

//
// free all the memory used by the game on the heap
//
void Connect4::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

//
// helper function for the winner check
//


Player* Connect4::checkForWinner()
{
    std::string state = stateString();
    auto get = [&](int x, int y) {
        return state[y * 7 + x];
    };

    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 7; x++) {
            char p = get(x, y);
            if (p == '0') continue; 

            if (x <= 7 - 4 &&
                p == get(x+1, y) &&
                p == get(x+2, y) &&
                p == get(x+3, y))
                return ownerAt(x, y);

            if (y <= 6 - 4 &&
                p == get(x, y+1) &&
                p == get(x, y+2) &&
                p == get(x, y+3))
                return ownerAt(x, y);

            if (x <= 7 - 4 && y <= 6 - 4 &&
                p == get(x+1, y+1) &&
                p == get(x+2, y+2) &&
                p == get(x+3, y+3))
                return ownerAt(x, y);

            if (x <= 7 - 4 && y >= 3 &&
                p == get(x+1, y-1) &&
                p == get(x+2, y-2) &&
                p == get(x+3, y-3))
                return ownerAt(x, y);
        }
    }

    return nullptr;
}


bool Connect4::checkForDraw()
{
    bool isDraw = true;
    // check to see if the board is full
    _grid->forEachSquare([&isDraw](ChessSquare* square, int x, int y) {
        if (!square->bit()) {
            isDraw = false;
        }
    });
    return isDraw;
}

//
// state strings
//
std::string Connect4::initialStateString()
{
    std::string s(42, '0');  // exactly 42 zeros
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Connect4::stateString()
{
    std::string s(42, '0');
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit *bit = square->bit();
        if (bit) {
            s[y * 7 + x] = std::to_string(bit->getOwner()->playerNumber()+1)[0];
        }
    });
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Connect4::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*7 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit( PieceForPlayer(playerNumber-1) );
        } else {
            square->setBit( nullptr );
        }
    });
}


//
// this is the function that will be called by the AI
//
void Connect4::updateAI() 
{
    int bestVal = -1000;
    int bestMove = NULL;
    std::string state = stateString();
    int* moves=getPossibleMoves(state);
    //_grid->forEachSquare([&](ChessSquare* square, int x, int y) {
    for(int x=0;x<7;x++){
        int y=moves[x];
        int index = (5-y) * 7 + x;
        if (state[index] == '0') {
            // Make the move
            state[index] = '2';
            int moveVal = -negamax(state, _gameOptions.AIMAXDepth, HUMAN_PLAYER);
            // Undo the move
            state[index] = '0';
            // If the value of the current move is more than the best value, update best
            if (moveVal > bestVal) {
                bestMove = x;
                bestVal = moveVal;
            }
        }
    };


    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(getHolderAt(bestMove,moves[bestMove]))) {
        }
    }
}

//
// player is the current player's number (AI or human)
//
int Connect4::negamax(std::string& state, int depth, int playerColor) 
{
    int score = evaluateAIBoard(state);

    // Check if AI wins, human wins, or draw
    if(score) { 
        // A winning state is a loss for the player whose turn it is.
        // The previous player made the winning move.
        return -score; 
    }

    if(isAIBoardFull(state)) {
        return 0; // Draw
    }

    if (depth == 3) {
        return -score;
    }
    
    int bestVal = -1000; // Min value
        for (int x = 0; x < 7; x++) {
            // Check if cell is empty
            

            auto moves=getPossibleMoves(state);
            int y=moves[x];
            if (state[(5-y) * 7 + x] == '0') {
                // Make the move
            
                state[(5-y) * 7 + x] = playerColor == HUMAN_PLAYER ? '1' : '2'; // Set the cell to the current player's color
                bestVal = std::max(bestVal, -negamax(state, depth + 1, -playerColor));
                // Undo the move for backtracking
                state[(5-y) * 7 + x] = '0';
            }
    }

    return bestVal;
}
bool Connect4::isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}

int Connect4::evaluateAIBoard(const std::string& state) 
{
    auto get = [&](int x, int y) {
        return state[y * 7 + x];
    };

    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 7; x++) {
            char p = get(x, y);
            if (p == '0') continue;

            if (x <= 7 - 4 &&
                p == get(x+1, y) &&
                p == get(x+2, y) &&
                p == get(x+3, y))
                return 1000;


            if (y <= 6 - 4 &&
                p == get(x, y+1) &&
                p == get(x, y+2) &&
                p == get(x, y+3))
                return 1000;

            if (x <= 7 - 4 && y <= 6 - 4 &&
                p == get(x+1, y+1) &&
                p == get(x+2, y+2) &&
                p == get(x+3, y+3))
                return 1000;

            if (x <= 7 - 4 && y >= 3 &&
                p == get(x+1, y-1) &&
                p == get(x+2, y-2) &&
                p == get(x+3, y-3))
                return 1000;
        }
    }

    return 0;
}

Player* Connect4::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 7 || y < 0 || y >= 6) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

int* Connect4::getPossibleMoves(std::string &state)
{
    static int moves[7];
     for(int i = 0; i < 7; i++) {
        moves[i] = -1;
    }
    for(int i=0;i<7;i++){
        int f=0;
        while(moves[i]==-1){
            int index=41;
            index= index-(f*7);
            index= index-(6-i);
             if(f==6){
                moves[i]=f;
                break;
            }
            if(state[index]=='0'&& index>-1){
                
                moves[i]=f;
            }
           
            f++;
        }
    }
    for(int i = 0; i < 7; i++) {
    }
    return moves;
}