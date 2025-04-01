import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileOutputStream;
import java.io.IOException;

public class PicLedController extends JFrame {
    
    private boolean ledState = false;
    private JButton toggleButton;
    private JLabel statusLabel;
    
    private static final String DEVICE_PATH = "/dev/pic_led";
    
    public PicLedController() {
        // Set up the frame
        setTitle("PIC LED Controller");
        setSize(300, 200);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        
        // Create components
        JPanel mainPanel = new JPanel();
        mainPanel.setLayout(new BorderLayout(10, 10));
        mainPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
        
        toggleButton = new JButton("Turn LED ON");
        toggleButton.setFont(new Font("Arial", Font.BOLD, 16));
        
        statusLabel = new JLabel("LED is currently OFF");
        statusLabel.setHorizontalAlignment(JLabel.CENTER);
        statusLabel.setFont(new Font("Arial", Font.PLAIN, 14));
        
        JPanel buttonPanel = new JPanel();
        buttonPanel.add(toggleButton);
        
        // Add components to frame
        mainPanel.add(statusLabel, BorderLayout.NORTH);
        mainPanel.add(buttonPanel, BorderLayout.CENTER);
        add(mainPanel);
        
        // Add action listener to button
        toggleButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                toggleLED();
            }
        });
    }
    
    private void toggleLED() {
        ledState = !ledState;
        String command = ledState ? "1" : "0";
        
        try {
            // Write command to the device
            FileOutputStream fos = new FileOutputStream(DEVICE_PATH);
            fos.write(command.getBytes());
            fos.close();
            
            // Update UI
            updateUI();
            
        } catch (IOException e) {
            JOptionPane.showMessageDialog(this,
                    "Error communicating with the device: " + e.getMessage(),
                    "Communication Error",
                    JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
        }
    }
    
    private void updateUI() {
        if (ledState) {
            toggleButton.setText("Turn LED OFF");
            statusLabel.setText("LED is currently ON");
        } else {
            toggleButton.setText("Turn LED ON");
            statusLabel.setText("LED is currently OFF");
        }
    }
    
    public static void main(String[] args) {
        // Run the GUI on the Event Dispatch Thread
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new PicLedController().setVisible(true);
            }
        });
    }
}