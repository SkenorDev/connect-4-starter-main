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
    _gameOptions.AIMAXDepth =5;
    _grid->initializeSquares(80, "square.png");


    startGame();
}
int Connect4::getLowestEmptyRow(int col)
{
    for (int row = 6 - 1; row >= 0; row--) {
        ChessSquare* square = _grid->getSquare(col, row);
        if (square && !square->bit()) {
            return row;
        }
    }
    return -1;
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
    int targetRow = getLowestEmptyRow(x);
    if (targetRow == -1) {
        return false;
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
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber() == 0 ? RED_PLAYER : YELLOW_PLAYER);

  
    if (bit) {
        ChessSquare* topSquare = _grid->getSquare(x, 0);
        ChessSquare* targetSquare = _grid->getSquare(x, targetRow);
        if (targetRow > 0) {
            bit->setPosition(topSquare->getPosition());
            bit->moveTo(targetSquare->getPosition());
        } else {
            bit->setPosition(targetSquare->getPosition());
        }
        targetSquare->setBit(bit);
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

int aiLowestRow(const std::string& state, int col) {
    for (int row = 6 - 1; row >= 0; row--) {
        char piece = state[row*7+col];
        if (piece == '0') {
            return row;
        }
    }
    return -1;  
}

//
// this is the function that will be called by the AI
//
void Connect4::updateAI() 
{
    int bestVal = -1000;
    BitHolder* bestMove = nullptr;
    std::string state = stateString();
    //_grid->forEachSquare([&](ChessSquare* square, int x, int y) {
    for(int x=0;x<7;x++){
        if (state[x] == '0') {
            int lowestRow = aiLowestRow(state, x);
            if (lowestRow > 5)
            continue;
            int index = lowestRow * 7 + x;
            // Make the move
            state[index] = '2';
            int moveVal = -negamax(state, 0, HUMAN_PLAYER);
            // Undo the move
            state[index] = '0';
            // If the value of the current move is more than the best value, update best
            if (moveVal > bestVal) {
                bestMove = _grid->getSquare(x,lowestRow);
                bestVal = moveVal;
            }
        }
    };


    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
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

    if (depth == 5) {
        return -score;
    }
    
    int bestVal = -1000; // Min value
        for (int x = 0; x < 7; x++) {
            // Check if cell is empty
            
        // Check if a move is possible in this column
        if (state[x] == '0') {
            int lowestRow = aiLowestRow(state, x);
            int index = lowestRow * 7 + x;
            // Make the move
        
            state[index] = playerColor == HUMAN_PLAYER ? '1' : '2'; // Set the cell to the current player's color
            bestVal = std::max(bestVal, -negamax(state, depth + 1, -playerColor));
            // Undo the move for backtracking
            state[index] = '0';
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
