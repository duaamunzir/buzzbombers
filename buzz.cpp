#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace sf;

// Initializing Dimensions
const int resolutionX = 960;
const int resolutionY = 640;
const int boxPixelsX = 32;
const int boxPixelsY = 32;
const int gameRows = resolutionY / boxPixelsY; // Total rows on grid
const int gameColumns = resolutionX / boxPixelsX; // Total columns on grid

// Configurable number of bees
const int maxBees = 20; // Number of bees that can exist at once
// Score setup
int score = 0;
// Array to track if a bee has been spawned or hit
bool beeActive[maxBees] = { false };
bool beePaused[maxBees] = { false };  // Track paused bees (hit by bullet)

// Max obstacles and tracking
const int maxObstacles = 100;
bool obstacleActive[maxObstacles] = { false };
float obstacleX[maxObstacles] = { 0 };
float obstacleY[maxObstacles] = { 0 };
int obstacleCount = 0;



// Function Declarations
// Track sprays and bullets fired
int spraysAvailable = 3;   // Configurable sprays (start with 3)
int bulletsFired = 0;      // Number of bullets fired

Sprite hummingbirdSprite;
Texture hummingbirdTexture;
float hummingbirdX = -100; // Off-screen initially
float hummingbirdY = -100; // Off-screen initially
bool hummingbirdActive = false;
Clock hummingbirdClock; // Timer to control hummingbird spawn
float lastHummingbirdSpawnTime = 0;
bool hummingbirdLanded = false; // Track if the hummingbird has landed
int hummingbirdFlightDuration = 5; // Duration for hummingbird flight (seconds)

Texture honeycombTexture;


void menu(RenderWindow& window, Font& font);
void drawPlayer(RenderWindow& window, float& player_x, float& player_y, Sprite& playerSprite);
void moveBullet(float& bullet_y, bool& bullet_exists, Clock& bulletClock);
void drawBullet(RenderWindow& window, float& bullet_x, float& bullet_y, Sprite& bulletSprite);
void spawnBees(Sprite beeSprites[], float beeX[], float beeY[], int beeDirection[], Clock& beeClock, float& lastSpawnTime);
void moveBees(Sprite beeSprites[], float beeX[], float beeY[], int beeDirection[]);
void drawObstacles(RenderWindow& window, Texture& obstacleTexture);
bool checkBulletHit(float bullet_x, float bullet_y, float& beeX, float& beeY);
bool checkCollisionWithObstacles(float player_x, float player_y);
void drawHummingbird(RenderWindow& window);
void spawnHummingbird(float player_x, float player_y);
void moveHummingbird(float player_x, float player_y);


int main() {
    srand(time(0));

    // Declaring RenderWindow.
    RenderWindow window(VideoMode(resolutionX, resolutionY), "Buzz Bombers", Style::Close | Style::Titlebar);
    window.setPosition(Vector2i(960, 64));

    // Initializing Background Music.
    Music bgMusic;
    if (!bgMusic.openFromFile("Music/Music3.ogg")) {
        cout << "Error: Could not load music file!" << endl;
    }

    Font font;
    if (!font.loadFromFile("Roboto/Roboto-Regular.ttf")) {
        cout << "Error loading font!" << endl;
    }

    menu(window , font);

    hummingbirdTexture.loadFromFile("Textures/bird.png");
    hummingbirdSprite.setTexture(hummingbirdTexture);
    hummingbirdSprite.setScale(0.5f, 0.5f); // Adjust scale
    hummingbirdLanded = true;
    // Obstacle texture
    Texture obstacleTexture;
    // Load obstacle texture
    obstacleTexture.loadFromFile("Textures/obstacles.png");

    bgMusic.setVolume(50);
    bgMusic.setLoop(true);
    bgMusic.play();

    // Initializing Player and Player Sprites
    float player_x = (resolutionX - boxPixelsX) / 2;
    float player_y = resolutionY - 64 - boxPixelsY;

    Texture playerTexture;
    Sprite playerSprite;
    playerTexture.loadFromFile("Textures/spray.png");
    playerSprite.setTexture(playerTexture);
    playerSprite.setPosition(player_x, player_y);
    // playerSprite.setTextureRect(IntRect(0, 0, boxPixelsX, boxPixelsY));
    playerSprite.setScale(0.5f, 0.5f);

    // Initializing Bullet and Bullet Sprites
    float bullet_x = player_x;
    float bullet_y = player_y;
    bool bullet_exists = false;

    Clock bulletClock;
    Texture bulletTexture;
    Sprite bulletSprite;
    bulletTexture.loadFromFile("Textures/bullet.png");
    bulletSprite.setTexture(bulletTexture);
    bulletSprite.setScale(3, 3);
    bulletSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));

    // The ground on which player moves
    RectangleShape groundRectangle(Vector2f(960, 64));
    groundRectangle.setPosition(0, (gameRows - 2) * boxPixelsX);
    groundRectangle.setFillColor(Color::Green);

    // Bee variables (fixed size arrays instead of vectors)
    Sprite beeSprites[maxBees];     // Bee sprite array
    float beeX[maxBees];            // Bee X positions
    float beeY[maxBees];            // Bee Y positions
    int beeDirection[maxBees];      // Bee movement direction (1 for right, -1 for left)
    Clock beeClock;
    float lastSpawnTime = 0;
    Texture beeTexture;
    beeTexture.loadFromFile("Textures/Regular_bee.png");
    for (int i = 0; i < maxBees; i++) {
        beeSprites[i].setTexture(beeTexture);
        beeSprites[i].setRotation(180);
        // beeSprites[i].setScale(2.5f,1.5f);
        // beeSprites[i].setColor(Color::Transparent);
    }


    // Honeycomb texture (this will be used for the paused bees)
    
    honeycombTexture.loadFromFile("Textures/honeycomb.png");  // Add your honeycomb image here

    
    
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::White);

    
    Sprite spraySprite;
    spraySprite.setTexture(playerTexture);

    
    // spraySprite.setScale(1.0f, 1.0f);// Resize spray icon if needed

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                return 0;
            }
        }

        // Player movement logic
        if (Keyboard::isKeyPressed(Keyboard::Left) && player_x > 0 && !checkCollisionWithObstacles(player_x - 5, player_y)) {
            player_x -= 5; // Move left
        }
        if (Keyboard::isKeyPressed(Keyboard::Right) && player_x < resolutionX  - 35 && !checkCollisionWithObstacles(player_x + 5, player_y)) {
            player_x += 5; // Move right
        }

        // Bullet firing logic: Fire bullet when spacebar is pressed
        if (Keyboard::isKeyPressed(Keyboard::Space) && !bullet_exists && spraysAvailable > 0) {
            
            bullet_x = player_x;  // Set the bullet position to player's current position
            bullet_y = player_y;  // Bullet starts from player's y position
            bullet_exists = true;  // Bullet exists now
            bulletClock.restart(); // Restart the bullet movement clock
            bulletsFired++;       // Increment bullets fired

            // Check if 56 bullets have been fired and reduce spray count
            if (bulletsFired >= 56) {
                spraysAvailable--;    // Decrease available sprays
                bulletsFired = 0;     // Reset bullet count
            }
        }

        // Check for bullet collision with bees
        if (bullet_exists) {
            for (int i = 0; i < maxBees; i++) {
                if (beeActive[i] && !beePaused[i]) {
                    if (checkBulletHit(bullet_x, bullet_y, beeX[i], beeY[i])) {
                        // Change bee to honeycomb and stop it
                        beeSprites[i].setTexture(honeycombTexture);
                        beeSprites[i].setScale(0.5f, 0.5f);
                        beeSprites[i].setTextureRect(sf::IntRect(0, 0, 100, 100));
                        score += 100;
                        beePaused[i] = true;  // Mark bee as paused (hit by bullet)
                        bullet_exists = false;  // Bullet disappears after hitting a bee

                        if (hummingbirdLanded ) {
                            hummingbirdX = beeX[i];
                            hummingbirdY = beeY[i];
                            hummingbirdLanded = false;
                        }
                        
                        break;  // Only allow the first hit to register
                    }
                }
            }
        }
        window.clear();
        // Spawn and move bees
        spawnBees(beeSprites, beeX, beeY, beeDirection, beeClock, lastSpawnTime);
        moveBees(beeSprites, beeX, beeY, beeDirection);

        

        // Update everything
        if (bullet_exists == true) {
            moveBullet(bullet_y, bullet_exists, bulletClock);
            drawBullet(window, bullet_x, bullet_y, bulletSprite);
        }

        drawPlayer(window, player_x, player_y, playerSprite);

        // Draw all bees
        for (int i = 0; i < maxBees; i++) {
            if (beeActive[i]) { // Only draw active and not paused bees
                window.draw(beeSprites[i]);
            }
        }

        // Draw the ground
        window.draw(groundRectangle);

        // Display the score after drawing all game objects
        scoreText.setString("Score: " + to_string(score));
        scoreText.setPosition(resolutionX - 150, resolutionY - 50);
        window.draw(scoreText); // Draw the score on top of the ground

        drawObstacles(window, obstacleTexture);

        // Display the spray icons
        for (int i = 0; i < spraysAvailable; i++) {
            spraySprite.setScale(0.5f, 0.5f);
            spraySprite.setPosition(30 * i , resolutionY - 50);
            window.draw(spraySprite); // Draw each spray icon
        }

        spawnHummingbird(player_x, player_y);
        moveHummingbird(player_x, player_y);
        drawHummingbird(window);
        // Display everything on screen
        window.display();
        
    }

}

void menu(RenderWindow& window, Font& menufont){
        
	
    Text text_tostart(" Press Enter to Start Game",menufont,50);
    text_tostart.setPosition(resolutionX/2 - 250,resolutionY/2 - 100);
    text_tostart.setFillColor(Color::Green);

    Text text_toexit("Exit",menufont,50);
    text_toexit.setPosition(resolutionX/2,resolutionY/2 + 50);
    text_toexit.setFillColor(Color::Red);

    while (window.isOpen()){
        Event e;
        while( window.pollEvent(e)){
        if(e.type == Event::Closed){
                window.close();
        }
        
        if(e.type == Event::KeyPressed){
                if(e.key.code == Keyboard::Enter){
                    return;
                }
                else if(e.key.code == Keyboard::Escape){
                    window.close();
                }
            }
        }
        
        window.clear(Color::Black);
        window.draw(text_tostart);
        window.draw(text_toexit);

        // To display the updated menu
        window.display();
    }
} 

void drawPlayer(RenderWindow& window, float& player_x, float& player_y, Sprite& playerSprite) {
    playerSprite.setPosition(player_x, player_y);
    window.draw(playerSprite);
}

void moveBullet(float& bullet_y, bool& bullet_exists, Clock& bulletClock) {
    if (bulletClock.getElapsedTime().asMilliseconds() < 20)
        return;

    bulletClock.restart();
    bullet_y -= 10; // Move bullet up
    if (bullet_y < -32) // If the bullet goes off-screen, remove it
        bullet_exists = false;
}

void drawBullet(sf::RenderWindow& window, float& bullet_x, float& bullet_y, Sprite& bulletSprite) {
    bulletSprite.setPosition(bullet_x, bullet_y);
    window.draw(bulletSprite);
}

void spawnBees(Sprite beeSprites[], float beeX[], float beeY[], int beeDirection[], Clock& beeClock, float& lastSpawnTime) {
    // Random spawn interval (between 1 and 3 seconds)
    float currentTime = beeClock.getElapsedTime().asSeconds();

    if (currentTime - lastSpawnTime > (rand() % 3 + 1)) {
        // Spawn a new bee in the top-left corner
        for (int i = 0; i < maxBees; i++) {
            if (!beeActive[i] && !beePaused[i]) { // Check if the bee hasn't been spawned yet
                // Bee texture

                // Set starting position to top-left (0, 0)
                beeX[i] = 10;
                beeY[i] = 0; // All bees spawn from the top-left corner (0, 0)

                // Set direction to move right
                beeSprites[i].setPosition(beeX[i], beeY[i]);
                beeDirection[i] = 1; // Moving right initially

                beeActive[i] = true; // Mark bee as active
                lastSpawnTime = currentTime; // Update last spawn time
                break;
            }
        }
    }
}


bool isBeeOverlappingWithObstacle(float beeX, float beeY, float obstacleX[], float obstacleY[], int maxObstacles) {
    // Iterate through existing obstacles and check if the bee position overlaps
    for (int i = 0; i < maxObstacles; i++) {
        if (obstacleX[i] == beeX && obstacleY[i] == beeY) {
            return true; // Bee is overlapping with an obstacle
        }
    }
    return false; // No overlap with any obstacle
}

bool spawnObstaclesWhenBeeHitsGround(int beeX, int beeY) {
 
    if (beeY >= resolutionY - 96) {
        // Check if the bee's position doesn't overlap with an existing obstacle
        if (!isBeeOverlappingWithObstacle(beeX, beeY, obstacleX, obstacleY, maxObstacles)) {
            // Mark obstacle here
            if (obstacleCount < maxObstacles) {
                obstacleX[obstacleCount] = beeX;
                obstacleY[obstacleCount] = beeY;
                obstacleActive[obstacleCount] = true;
                obstacleCount++;

                return true;
            }
        }
    }
    return false;
    
}

void moveBees(Sprite beeSprites[], float beeX[], float beeY[], int beeDirection[]) {
    for (int i = 0; i < maxBees; i++) {
        if (beeActive[i] && !beePaused[i]) { // Only move active and not paused bees
            // Move bee horizontally
            beeX[i] += 5 * beeDirection[i]; // Move 2 pixels per frame

            // If bee reaches the right edge, turn it around and drop vertically
            if (beeX[i] > resolutionX - boxPixelsX) {
                beeDirection[i] = -1; // Turn left
                beeY[i] += boxPixelsY; // Drop height by 1 step
                beeSprites[i].setRotation(0);
            }

            // If bee reaches the left edge, turn it around and drop vertically
            if (beeX[i] < 0) {
                beeDirection[i] = 1; // Turn right
                beeY[i] += boxPixelsY; // Drop height by 1 step
                beeSprites[i].setRotation(180);
            }

            // Update bee position
            beeSprites[i].setPosition(beeX[i], beeY[i]);
            
            // beeSprites[i].setRotation( beeDirection[i] * 180);
            if (spawnObstaclesWhenBeeHitsGround(beeX[i], beeY[i])) {
                beeActive[i] = false;
                beePaused[i] = true;
            }

        } else if(beeActive[i] && beeSprites[i].getTexture() == &honeycombTexture) {
            float hummingbirdX = hummingbirdSprite.getPosition().x;
            float hummingbirdY = hummingbirdSprite.getPosition().y;
            if (hummingbirdX >= beeX[i] - boxPixelsX && hummingbirdX <= beeX[i] + boxPixelsX && 
                hummingbirdY >= beeY[i] - boxPixelsY && hummingbirdY <= beeY[i] + boxPixelsY) {
                // If hummingbird lands on honeycomb, remove the honeycomb
                // beeSprites[i].setTexture(hummingbirdTexture); // Set texture to empty
                beeSprites[i].setColor(Color::Transparent); // Make it invisible
                beeActive[i] = false;
                score += 1000; 
            }
        }
    }
}

// Function to check if bullet hit the bee
bool checkBulletHit(float bullet_x, float bullet_y, float& beeX, float& beeY) {
    // Check if the bullet intersects with the bee's area
    return (bullet_x > beeX - boxPixelsX && bullet_x < beeX + boxPixelsX && bullet_y > beeY && bullet_y < beeY + boxPixelsY);
}

// Function to check collision with obstacles
bool checkCollisionWithObstacles(float player_x, float player_y) {
    for (int i = 0; i < obstacleCount; i++) {
        if (obstacleActive[i]) {
            // Check if player position collides with any obstacle
            if (player_x >= obstacleX[i] - boxPixelsX && player_x <= obstacleX[i] + boxPixelsX &&
                player_y >= obstacleY[i] && player_y <= obstacleY[i] + boxPixelsY) {
                return true;
            }
        }
    }
    return false;
}

void drawObstacles(RenderWindow& window, Texture& obstacleTexture) {
    for (int i = 0; i < obstacleCount; i++) {
        if (obstacleActive[i]) {
            Sprite obstacleSprite;
            obstacleSprite.setTexture(obstacleTexture);
            obstacleSprite.setPosition(obstacleX[i], obstacleY[i]);

            window.draw(obstacleSprite);
        }
    }
}

void drawHummingbird(RenderWindow& window) {
    if (hummingbirdActive) {
        window.draw(hummingbirdSprite);  // Draw the hummingbird
    }
}

void spawnHummingbird(float player_x, float player_y) {
    float currentTime = hummingbirdClock.getElapsedTime().asSeconds();

    if (currentTime - lastHummingbirdSpawnTime > (rand() % 6 + 5)) {
        hummingbirdX = rand() % (resolutionX - boxPixelsX);  // Random X within the screen width
        hummingbirdY = rand() % (gameRows / 2) * boxPixelsY; // Random Y position above the player

        hummingbirdActive = true;
        lastHummingbirdSpawnTime = currentTime;
        hummingbirdLanded = false;
        hummingbirdClock.restart();
    }
}


void moveHummingbird(float player_x, float player_y) {
    if (hummingbirdActive) {
        float deltaX = hummingbirdX - hummingbirdSprite.getPosition().x;
        float deltaY = hummingbirdY - hummingbirdSprite.getPosition().y;
        if (deltaX != 0 || deltaY == 0) { 
             float moveX = 1;
             if (deltaX < 0) {
                moveX = -1;
             }

             float moveY = 1;
             if (deltaY < 0) {
                moveY = -1;
             }

            // Move hummingbird towards target
            hummingbirdSprite.move(moveX, moveY); // Move at constant speed
        } else {
            // Reset hummingbird after landing
            hummingbirdLanded = true;
            spawnHummingbird(player_x, player_y); // Spawn a new target
            
        }
    }
}

