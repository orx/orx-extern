
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the main rendering window
    sf::RenderWindow App(sf::VideoMode(800, 600, 32), "SFML Graphics");
	App.SetFramerateLimit(60);

    // Start game loop
    while (App.IsOpened())
    {
        // Process events
        sf::Event Event;
        while (App.GetEvent(Event))
        {
            // Close window : exit
            if (Event.Type == sf::Event::Closed)
                App.Close();

            // A key has been pressed
            if (Event.Type == sf::Event::KeyPressed)
            {
                // Escape key : exit
                if (Event.Key.Code == sf::Key::Escape)
                    App.Close();

                // F1 key : capture a screenshot
                if (Event.Key.Code == sf::Key::F1)
                {
                    sf::Image Screen = App.Capture();
                    Screen.SaveToFile("screenshot.jpg");
                }
            }
        }

        // Clear the screen with red color
        App.Clear(sf::Color(200, 0, 0));

        // Display window contents on screen
        App.Display();
    }

    return EXIT_SUCCESS;
}
