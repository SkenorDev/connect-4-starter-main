#pragma once
#include "Game.h"

//
// the classic game of tic tac toe
//

//
// the main game class
//
class Connect4 : public Game
{
public:
    Connect4();
    ~Connect4();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void        stopGame() override;
    int evaluateAIBoard(const std::string& state);
    bool isAIBoardFull(const std::string& state);
	void        updateAI() override;
    bool        gameHasAI() override { return true; }
    Grid* getGrid() override { return _grid; }
    Player* ownerAt(int x, int y) const;
    int* getPossibleMoves(std::string &s);
private:
    static const int EMPTY = 0;
    static const int RED_PIECE = 1;
    static const int YELLOW_PIECE = 2;

    static const int RED_PLAYER = 0;
    static const int YELLOW_PLAYER = 1;
    Bit *       PieceForPlayer(const int playerNumber);
    int         negamax(std::string& state, int depth, int playerColor);

    Grid*       _grid;
};

