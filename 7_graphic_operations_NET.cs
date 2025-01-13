using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Drawing;        // od grafiki
using System.Windows.Forms;  // od okien

namespace A
{
    public class BallAnimationForm : Form
    {
        private Timer timer;
        private int ballX = 50, ballY = 50, ballSpeedX = 5, ballSpeedY = 5;
        private int ballRadius = 20;
        private Rectangle obstacle1, obstacle2;

        public BallAnimationForm()
        {
            this.Text = "Piłka Animacja z Przeszkodami";
            this.Size = new Size(800, 600); 
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.StartPosition = FormStartPosition.CenterScreen;

            // przeszkody
            obstacle1 = new Rectangle(300, 200, 100, 20);
            obstacle2 = new Rectangle(500, 400, 150, 20);

            // timer do animacji
            timer = new Timer();
            timer.Interval = 16; // 60 FPS
            timer.Tick += new EventHandler(Update);
            timer.Start();
        }

        // funkcja do rysowania 
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            Graphics g = e.Graphics;

            // rysowanie figur
            g.DrawLine(Pens.Black, 100, 100, 700, 100); // linia
            g.DrawRectangle(Pens.Red, 100, 150, 150, 100); // prostokąt
            g.FillEllipse(Brushes.Green, 500, 100, 100, 100); // koło

            // rysowanie przeszkód
            g.FillRectangle(Brushes.Blue, obstacle1);
            g.FillRectangle(Brushes.Blue, obstacle2);

            // rysowanie piłki
            g.FillEllipse(Brushes.Orange, ballX - ballRadius, ballY - ballRadius, ballRadius * 2, ballRadius * 2);
        }

        // funkcja do aktualizacji animacji
        private void Update(object sender, EventArgs e)
        {
            // ruch piłki
            ballX += ballSpeedX;
            ballY += ballSpeedY;

            // odbicie od krawędzi okna
            if (ballX - ballRadius < 0 || ballX + ballRadius > this.ClientSize.Width)
            {
                ballSpeedX = -ballSpeedX; // zmiana kierunku poziomego
            }
            if (ballY - ballRadius < 0 || ballY + ballRadius > this.ClientSize.Height)
            {
                ballSpeedY = -ballSpeedY; // zmiana kierunku pionowego
            }

            // odbicie od przeszkód (kolizja z prostokątami)
            Rectangle ballRect = new Rectangle(ballX - ballRadius, ballY - ballRadius, ballRadius * 2, ballRadius * 2);
            if (ballRect.IntersectsWith(obstacle1) || ballRect.IntersectsWith(obstacle2))
            {
                ballSpeedY = -ballSpeedY; // zmiana kierunku pionowego
            }

            
            this.Invalidate();
        }

        
        public static void Main()
        {
            Application.Run(new BallAnimationForm());
        }
    }
}
