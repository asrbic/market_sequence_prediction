package crypto.poloniex.wamp.swing;

import java.awt.Button;
import java.awt.Component;
import java.awt.Container;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;
import java.util.Map;

import javax.swing.JFrame;
import javax.swing.JTable;
import javax.swing.ScrollPaneLayout;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;

import rx.functions.Action1;
import sun.swing.DefaultLayoutStyle;
import ws.wamp.jawampa.PubSubData;
import ws.wamp.jawampa.WampClient;
import ws.wamp.jawampa.WampClient.State;

public class PoloWAMPGUI extends JFrame {
	//Map<String, TextField> comps;
	public Label connectionStatus;
	public JTable tickerTable;
	
	public PoloWAMPGUI(ActionListener actionListener, TableModel model, TableColumnModel colModel) {
		super("Crypro GUI");
		Container contentPane = getContentPane();
		contentPane.setLayout(new GridBagLayout());
		tickerTable = new JTable(model);//, colModel);
		tickerTable.setSize(1500, 800);
		contentPane.add(tickerTable);
		Button startButton = new Button("Start Price Stream");
		startButton.setName("START");
		Button stopButton = new Button("Stop Price Stream");
		stopButton.setName("STOP");
		startButton.addActionListener(actionListener);
		stopButton.addActionListener(actionListener);
		contentPane.add(startButton);
		contentPane.add(stopButton);
		connectionStatus = new Label("Disconnected");
		contentPane.add(connectionStatus);
		setSize(1500, 1000);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
	}

}
