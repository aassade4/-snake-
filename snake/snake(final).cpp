/* 要实现贪吃蛇，首先要先明确我们需要的东西，1.蛇：蛇身(snake body)的增长,蛇头（snake head）的移动及方向的控制，蛇的死亡逻辑（从第四节开始碰撞自身会导致死亡，碰壁同理）
蛇尾（snake tail）的消失，蛇身的移动，蛇的运动速度,;2:食物的随机生成以及生成的逻辑,可以考虑bool 来实现随机生成,1=为生成,执行生成逻辑,0表示不生成
3:地图初始化边框逻辑和游戏结束的逻辑,可以先写出简单版本,随后利用图形库进一步的优化,也可以改进为鼠标控制方向的有意思模式 ,同时屏幕应该还处于相对稳定的状态,不应该出现频闪的情况*/

#include <iostream>
#include <conio.h>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <fstream>
#include <graphics.h>

const int CELL_SIZE = 20; //每个格子的大小
const int WIDTH = 50, HEIGHT = 50; //地图的大小
enum direction {UP, DOWN, LEFT, RIGHT};
ExMessage msg;

class snake{
private:
    std::vector<std::pair<int,int> > body; //蛇的位置,用body.front()表示蛇头，body.back()表示蛇尾
    int direction;//蛇头的方向
    int speed;//蛇的运动速度（和蛇长有关，初始三节为蛇的原始速度，后面每加一节就加速，速度不得高于原速十倍，可以考虑设定初速度，如何后面加速度就直接乘倍率）
    int score;//蛇的得分
    bool gen_food;//是否应该随机生成食物
    bool gameover;//是否结束游戏
    struct food{
        int x,y;
    }food;
    std::mt19937 gen;//随机种子
public:
// 构造函数：初始化蛇为3节（示例位置）
    snake() {
        gen.seed(std::time(nullptr)); // 关键修改：用当前时间当种子
        body.push_back({10, 5});  // 头
        body.push_back({9, 5});   // 中间节
        body.push_back({8, 5});   // 尾
        direction = RIGHT;  // 初始向右
        speed = 200;    // 初始延迟200ms
        score = 0;
        gen_food = true;
        gameover = false;
        genfood();
    }
    // 初始生成食物
    //大模块细分成小具体，pubilc函数只有声明，而private函数有具体实现，可以实现蛇的各种功能，如蛇的移动，蛇的死亡，食物的生成，地图的初始化等等。
    //蛇的移动,包括速度，蛇尾消失，蛇头出现，还有蛇身的移动

    //速度
    void updateSpeed(){
        int newspeed = speed - (score/30)*20;
        speed = std::max(newspeed, 50); // 速度不得高于50ms
    }
    void move(){
        //头的方向控制
        //auto head = body.front();
        auto head = body.front();
        int headx = head.first;
        int heady = head.second;

        //更改成适配图形库的输入
        // if(_kbhit()){
        //     char key = _getch();
        //     key = toupper(key);
        //     switch(key){
        //         //移动的合法性，不能出现一百八十度大转弯这个自杀情况
        //         case 'W': if(direction != DOWN) direction = UP; break;
        //         case 'S': if(direction != UP) direction = DOWN; break;
        //         case 'A': if(direction != RIGHT) direction = LEFT; break;
        //         case 'D': if(direction != LEFT) direction = RIGHT; break;
        //     }
        // }

        //非阻塞按键输入(这里的缓冲太大了，所以采用另一个方式，更触碰底层的多缓冲输入)
        // if(GetAsyncKeyState('W') & 0x8000 && direction != DOWN) direction = UP;
        // if(GetAsyncKeyState('S') & 0x8000 && direction != UP) direction = DOWN;
        // if(GetAsyncKeyState('A') & 0x8000 && direction != RIGHT) direction = LEFT;
        // if(GetAsyncKeyState('D') & 0x8000 && direction != LEFT) direction = RIGHT;

        while(peekmessage(&msg,EX_KEY)){
            if(msg.message == WM_KEYDOWN){
                switch (msg.vkcode){
                    case 'W': if(direction != DOWN) direction = UP; break;
                    case 'S': if(direction != UP) direction = DOWN; break;
                    case 'A': if(direction != RIGHT) direction = LEFT; break;
                    case 'D': if(direction != LEFT) direction = RIGHT; break;
                }
            }
        }

        switch(direction){
            case UP: heady--; break;
            case DOWN: heady++; break;
            case LEFT: headx--; break;
            case RIGHT: headx++; break;
        }
        //新头的插入
        body.insert(body.begin(),{headx,heady});
        //判断是否吃到
        if(headx == food.x && heady == food.y){
            score += 10;
            gen_food = true;//重新生成食物
            // 吃到食物不删尾节（实现增长）
            updateSpeed();   // 增长后更新速度
        } else {
        // 没吃到食物，删除尾节（保持长度）
            body.pop_back();
        }
    }
    //取速度
    int getspeed() const{
        return speed;
    }
    //取得分
    int getscore() const {
        return score;
    }

    void genfood(){
        //食物的生成,食物的生成是建立在地图这个二维数组的随机生成，1，可执行时候随机生成，0，不生成，且只有蛇吃下去后长度会增加，不会直接增加，不能生成在蛇身上和蛇头上
        if(!gen_food) return;

        std::uniform_int_distribution<int> xDist(1, WIDTH - 2);
        std::uniform_int_distribution<int> yDist(1, HEIGHT - 2);

        bool flag;
        do{
            flag = false;
            //生成随机位置
            food.x = xDist(gen);
            food.y = yDist(gen);
            //检验是否生成在蛇身上，对蛇身体进行位置的遍历
            for(const auto& p : body){
                if(p.first == food.x && p.second == food.y){
                    flag = true;
                    break;
                }
            }

        }while(flag);
    
        gen_food = false;
    }
    void Gameover(){
        //游戏结束的逻辑，蛇死亡，或者蛇撞墙，或者蛇撞自身，因为蛇三节以上才能撞死自己，所以撞自己的逻辑要求蛇长大于三，蛇撞墙则是蛇头抵达地图吧边界
        //结束的话应该在控制台输出游戏结束，并显示最终得分，然后选择按退出键或者是重新开始。
        //1.边界的检验
        auto head = body.front();
        int headx = head.first;
        int heady = head.second;
        if(headx<=0 || headx>=WIDTH-1 || heady<=0 || heady>=HEIGHT-1){
            gameover=true;
            return;
        }
        //2.蛇身的检验,从三节及以后开始
        if(body.size()>3){
            for(size_t i=1;i<body.size();++i){
                if(body[i].first == headx && body[i].second == heady){
                    gameover=true;
                    return;
                }
            }
        }
    }
    //获取游戏当前状态
    bool isGameover() const {
        return gameover;
    }   
    //重置游戏状态
    void reset() {
        body.clear();
        body.push_back({10, 5});
        body.push_back({9, 5});
        body.push_back({8, 5});
        direction = RIGHT;
        speed = 200;
        score = 0;
        gen_food = true;
        gameover = false;
        genfood();
    }

    friend void draw(snake& snake);
    friend bool issnakebody(snake& snake, int x, int y);
    friend void point(snake& snake);
    friend bool gamestart(snake& snake);
    friend bool close(snake& snake);
    
};

bool issnakebody(snake &snake,int x,int y){
    //蛇身的检查，蛇身的位置不计入蛇头的位置，所以要从第二节开始
    for (size_t i = 1; i < snake.body.size(); ++i) {
        if (snake.body[i].first == x && snake.body[i].second == y) {
            return true;
        }
    }
    return false;
}
//地图绘制
void draw(snake &snake){
    //清除频闪
    // system("cls");

    // //1.上边界
    // for(int i = 0;i < WIDTH; ++i){
    //     std::cout << "#";
    // }
    // std::cout << std::endl;

    // //2.绘制中间区域
    // for(int y = 1;y < HEIGHT -1; ++y){
    //     for(int x = 0;x < WIDTH; ++x){
    //         if(x == 0 || x == WIDTH-1){
    //             std::cout << "#";
    //         }else{
    //             //蛇头检查
    //             if(x == snake.body.front().first && y == snake.body.front().second){
    //                 std::cout << "Q";
    //             }
    //             //蛇身检查（不计蛇头
    //             else if(issnakebody(snake,x,y)){
    //                 std::cout << "o";
    //             }
    //             //食物检查
    //             else if(x == snake.food.x && y == snake.food.y){
    //                 std::cout << "*";
    //             }
    //             //空白区域
    //             else{
    //                 std::cout << " ";
    //             }
    //         }  
    //     }
    //     std::cout << std::endl;
    // }

    // //3.下边界
    // for (int i = 0; i < WIDTH; ++i){
    //     std::cout << "#";
    // }
    // std::cout << std::endl;

    // //显示得分和速度
    // std:: cout << "Score:" << snake.getscore() << " " << "Speed:" << snake.getspeed() << "ms" << std::endl;
    static bool first_draw = true;
    if(first_draw){
        cleardevice(); //清屏
        first_draw = false;
    }

    BeginBatchDraw(); //批量绘制
    cleardevice(); //清屏

    //边框绘制
    setlinecolor(BLACK);
    rectangle(0,0,WIDTH*CELL_SIZE,HEIGHT*CELL_SIZE);

    //蛇绘制
    for(size_t i = 0; i < snake.body.size();++i){
        int x = snake.body[i].first*CELL_SIZE;
        int y = snake.body[i].second*CELL_SIZE;
        setfillcolor(i == 0? RED : GREEN);
        solidrectangle(x+1,y+1,x+CELL_SIZE-1,y+CELL_SIZE-1);
    }

    //食物绘制
    setfillcolor(BLUE);
    solidrectangle(snake.food.x*CELL_SIZE+1,snake.food.y*CELL_SIZE+1,(snake.food.x+1)*CELL_SIZE-1,(snake.food.y+1)*CELL_SIZE-1);
    
    //分数和速度绘制
    static char scoreText[50];
    sprintf(scoreText, "Score:%d Speed:%dms", snake.getscore(), snake.getspeed());
    setcolor(BLACK);
    outtextxy(10,HEIGHT*CELL_SIZE-30,(LPCTSTR)scoreText);

    EndBatchDraw(); //结束批量绘制
}   

void point(snake &snake){
    //记录最高分
    static int highscore = 0;
    //1.读取最高分（如果文件存在
    std::ifstream in("Score.txt");
    if(in.is_open()){
        in >> highscore;
        in.close();
    }
    //2.比较最高分和当前分，更新最高分
    if(snake.getscore() > highscore){
        highscore = snake.getscore();
        std::cout << "最高分:" << highscore << std::endl;
        std::ofstream out("Score.txt");//打开文件写入数据（不存在则创建）
        out << highscore;
        out.close();
    }
}

// bool gamestart(snake &snake){
//     while(!snake.isGameover()){
//         snake.move();
//         snake.genfood();
//         snake.Gameover();
//         draw(snake);
//         Sleep(snake.getspeed());
//     }
//     return true;
// }

// bool close(snake &snake){
//     //1.游戏结束，显示游戏结束，并显示最终得分
//     std::cout << "Game Over!" << std::endl;
//     std::cout << "Score:" << snake.getscore() << std::endl;
//     point(snake);
//     //2.选择是否重新开始
//     char key;
//     do{
//         std::cout << "按Q退出游戏,按R键重新开始: ";
//         key = _getch();
//         key = toupper(key);
//     }while(key != 'Q' && key != 'q' && key != 'R' && key != 'r');
//     if(key == 'R' || key == 'r'){
//         snake.reset();  // 重置游戏状态
//         return true;    // 重新开始
//     }
//     //3.退出游戏
//     return false;
// }

int main()
{
    //初始化图形窗口
    initgraph(WIDTH*CELL_SIZE, HEIGHT*CELL_SIZE+40); //初始化窗口
    setbkcolor(WHITE); //设置背景颜色
    cleardevice(); //清屏

    // 设置控制台代码页为UTF-8，解决中文乱码问题
    // SetConsoleOutputCP(CP_UTF8);

    snake game_snake;
    bool restart = false;
    int highscore = 0;//最高分存储

    //帧时间控制变量
    DWORD last_time = GetTickCount();
    const int FPS = 60; //帧率
    const int FRAME_TIME = 1000 / FPS; //每帧时间间隔

    while(true){
        // 检查游戏是否结束
        if(game_snake.isGameover()){
            //游戏结束，显示游戏结束，并显示最终得分
            BeginBatchDraw(); //批量绘制
            cleardevice(); //清屏
            
            //显示游戏结束文字
            setcolor(BLACK);
            outtextxy(50,50,(LPCTSTR)"Game Over!");
            outtextxy(50,80,(LPCTSTR)"Press R to restart, press Q to exit");

            //显示最终得分
            char scoreText[50];
            sprintf(scoreText, "final point:%d", game_snake.getscore());
            outtextxy(50,100,(LPCTSTR)scoreText);

            //显示最高分
            std::ifstream in("Score.txt");
            if(in.is_open()){
                in >> highscore;
                in.close();
            }
            char highText[50];
            sprintf(highText, "highest point:%d", highscore);
            outtextxy(50,130,(LPCTSTR)highText);

            EndBatchDraw(); //结束批量绘制

            //输入处理（输入R,Q决定重开或退出）
            bool keyPressed = false;
            ExMessage turn;
            while(!keyPressed){
                // 持续检查是否有按键消息
                while(peekmessage(&turn,EX_KEY)){
                    if(turn.message == WM_KEYDOWN){
                        // 确保正确处理大写和小写字母
                        if(turn.vkcode == 'Q' || turn.vkcode == 'q' || turn.vkcode == VK_ESCAPE){
                            restart = false;
                            keyPressed = true;
                        }
                        else if(turn.vkcode == 'R' || turn.vkcode == 'r'){
                            restart = true;
                            keyPressed = true;
                        }
                    }
                }
                // 短暂延迟，避免CPU占用过高
                Sleep(10);
            }

            // 根据用户选择决定是否重启游戏
            if(restart){
                game_snake.reset();
            } else {
                break; // 退出游戏循环
            }
        } else {
            // 游戏进行中
            //计算时间差
            DWORD current_time = GetTickCount();
            DWORD delta_time = current_time - last_time;

            if(delta_time >= FRAME_TIME){
                last_time = current_time;

                game_snake.move();
                game_snake.genfood();
                game_snake.Gameover();

                //调用Point函数，更新最高分
                point(game_snake);

                //调用draw函数，绘制游戏界面
                draw(game_snake);
                Sleep(game_snake.getspeed());
            }
        }
    }

    //关闭图形窗口
    closegraph();
    return 0;
}