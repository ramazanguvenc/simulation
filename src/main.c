#include <stdio.h>
#include "raylib.h"
#include "queue.h"
#include "client.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(A[0]))

enum {
    MENU_NEW = 0,
    MENU_OPEN,
    MENU_SAVE,
    MENU_SAVEAS,
    MENU_EXIT,
};

enum {
    SUBMENU_NEW_PROJECT=0,
    SUBMENU_NEW_DATABASE,
    SUBMENU_NEW_WORKSPACE,
};

enum {
    SUBMENU_OPEN_FILE = 0,
    SUBMENU_OPEN_PROJECT,
};

enum {
    SUBMENU_SAVEAS_IMAGE = 0,
    SUBMENU_SAVEAS_DATABASE,
    SUBMENU_SAVEAS_CPP,
    SUBMENU_SAVEAS_JSON
};

enum {
    STATE_NORMAL = 0, 
    STATE_SHOW_MENU,
    STATE_SHOW_SUBMENU,
};


node_t *queue = NULL;
int thread_flag = 1;

void* connection_thread(void *vargp);
int new();
void try_connect();

char ip[30];
char str_port[30];
int port = 12300; //default port

int main(int argc, char **argv)
{
    if(argc == 3){
        strcpy(ip, argv[1]);
        strcpy(str_port, argv[2]);
        port = atoi(str_port);
    }
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "menu");
    SetTargetFPS(60);
    
    int state = STATE_NORMAL;
    int mainActive = -1, mainFocused = -1;
    int subActive = -1;
    

    const char* menuItems[] = { "New", "Exit" };
    Rectangle menuRec = {450, 300, 450, 700};
    state = STATE_SHOW_MENU;

    while (!WindowShouldClose())
    {

        // UPDATE
        Vector2 mouse = GetMousePosition();
        // -------------
		BeginDrawing();
		ClearBackground(LIGHTGRAY);
        DrawText("SELECT NEW FOR STARTING SIMULATION!", 457, 250, 20, BLACK);
        
        
        int sz = 0;
        const int itemHeight = (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)+GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
        if(state == STATE_SHOW_MENU)
        {
            menuRec.height = itemHeight * ARRAY_SIZE(menuItems) + 10; // calculate menu height based on how many items it has
            int focused = mainFocused;
            mainActive = GuiListViewEx(menuRec, menuItems, ARRAY_SIZE(menuItems), &focused, NULL, mainActive);
            if(focused != -1)  { 
                mainFocused = mainActive = focused;
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if(strcmp(menuItems[focused], "New") == 0)new();
                    else if(strcmp(menuItems[focused], "Exit") == 0){
                        EndDrawing();
                        CloseWindow();
                        return 0;
                    }
                    mainFocused = mainActive = subActive = -1;
                    
                }
            }

        }        
		EndDrawing();
	}

	CloseWindow();
    printf("EndOfProgram");

	return 0;
}

int status = -1;

int new(void)
{

    

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1366;
    const int screenHeight = 768;

    //InitWindow(screenWidth, screenHeight, "simulation");

    // Define the camera to look into our 3d world
    Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

    Vector3 playerPosition = { 0.0f, 0.0f, 0.0f };
    Color playerColor = GREEN;
    float playerSize = 0.5f;


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    //set variables for input box.
    int MAX_INPUT_CHARS = 20;
    char my_IP[128] = "\0"; 
    
    int letterCount = 0;
    Rectangle textBox = {50, 170, 270, 40, 130};
    
    int mouseOnText = 0;
    int framesCounter = 0;


    Texture2D button = LoadTexture("resources/button.png"); // Load button texture

    // Define frame rectangle for drawing
    int NUM_FRAMES = 3;
    int frameHeight = button.height/NUM_FRAMES;
    Rectangle sourceRec = { 10, 220, button.width, frameHeight };

    // Define button bounds on screen
    Rectangle btnBounds = {10, 220, button.width, frameHeight };

    int btnState = 0;               // Button state: 0-NORMAL, 1-MOUSE_HOVER, 2-PRESSED
    int btnAction = 0;         // Button action should be activated

    Vector2 mousePoint = { 0.0f, 0.0f };



    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        
        mousePoint = GetMousePosition();
        btnAction = false;

        // Check button state
        if (CheckCollisionPointRec(mousePoint, btnBounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnState = 2;
            else btnState = 1;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) btnAction = true;
        }
        else btnState = 0;

        if(btnAction){
            strcpy(ip, my_IP);
            try_connect();
        }
        sourceRec.y = btnState*frameHeight;


        if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = 1;
        else mouseOnText = 0;

        if(mouseOnText){
            int key = GetKeyPressed();
            while(key > 0){
                if((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS)){
                    my_IP[letterCount] = (char) key;
                    letterCount++;
                    
                }

                key = GetKeyPressed();
            }
        }
        
        if(IsKeyPressed(KEY_BACKSPACE)){
            letterCount--;
            my_IP[letterCount] = '\0';
            if(letterCount < 0) letterCount = 0;
        }

        if(mouseOnText) framesCounter++;
        else framesCounter = 0;
       
        float x, y, z;
        x = dequeue(&queue);
        y = dequeue(&queue);
        z = dequeue(&queue);
        
        if(x != -66.6f){
            playerPosition.x = x;
            playerPosition.y = y;
            playerPosition.z = z;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawTextureRec(button, sourceRec, (Vector2){ 10, 220 }, WHITE); // Draw button frame

            DrawRectangleRec(textBox, LIGHTGRAY);
            if (mouseOnText) DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, RED);
            else DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, DARKGRAY);
            DrawText(my_IP, textBox.x + 5, textBox.y + 8, 40, MAROON);
            DrawText("IP:", 8, 173, 33, BLACK);
            if (mouseOnText)
            {
                if (letterCount < MAX_INPUT_CHARS)
                {
                    if (((framesCounter/20)%2) == 0) DrawText("_", textBox.x + 8 + MeasureText(my_IP, 40), textBox.y + 12, 40, MAROON);
                }
                else DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
            }

            if(status == -1) DrawText("Connection Failed", 10, 50, 20, RED);
            else DrawText("Connection Successful", 10, 50, 20, GREEN);
            DrawText("Press ESC to return", 10, 90, 20, BLACK);
            char msg[128]; sprintf(msg,"x = %.2f, y = %.2f, z = %.2f", playerPosition.x, playerPosition.y, playerPosition.z);
            DrawText(msg, 10, 140, 20, BLACK);

            BeginMode3D(camera);

                // Draw player
                DrawSphere(playerPosition, playerSize, playerColor);

                DrawGrid(10, 1.0f);        // Draw a grid

            EndMode3D();

            

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    //CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    thread_flag = 0;

    close_port();
    return 0;
}

void try_connect(){
    pthread_t worker_id;
    pthread_create(&worker_id, NULL, connection_thread, NULL);
}

void* connection_thread(void *vargp){
    thread_flag = 1;
    status = open_port(ip, port);
    if(status == -1) thread_flag = 0;
    
    while(thread_flag){
        char* buffer = get_data_from_server();
        
        if(buffer == NULL){}
        else{
            float x, y, z;
            sscanf(buffer, "%f %f %f", &x, &y, &z);
            if(queue_size < 1000){
                enqueue(&queue, x);
                enqueue(&queue, y);
                enqueue(&queue, z);
            }
            free(buffer);
        }
    }

    if(queue != NULL){
        while(dequeue(&queue) != -66.6f);
        free(queue);
        queue = NULL;
    }
    return NULL;
}


