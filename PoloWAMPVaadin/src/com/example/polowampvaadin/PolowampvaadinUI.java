package com.example.polowampvaadin;

import java.text.NumberFormat;
import java.util.Locale;
import java.util.Observable;
import java.util.Observer;

import javax.servlet.annotation.WebServlet;

import com.vaadin.annotations.Push;
import com.vaadin.annotations.Theme;
import com.vaadin.annotations.VaadinServletConfiguration;
import com.vaadin.client.widget.grid.CellReference;
import com.vaadin.client.widget.grid.CellStyleGenerator;
import com.vaadin.data.Container;
import com.vaadin.data.Container.Indexed;
import com.vaadin.data.Item;
import com.vaadin.data.Property;
import com.vaadin.data.util.GeneratedPropertyContainer;
import com.vaadin.data.util.IndexedContainer;
import com.vaadin.data.util.ObjectProperty;
import com.vaadin.data.util.PropertyValueGenerator;
import com.vaadin.data.util.converter.Converter;
import com.vaadin.data.util.converter.DefaultConverterFactory;
import com.vaadin.data.util.converter.StringToDoubleConverter;
import com.vaadin.server.VaadinRequest;
import com.vaadin.server.VaadinServlet;
import com.vaadin.server.VaadinSession;
import com.vaadin.ui.Button;
import com.vaadin.ui.HorizontalLayout;
import com.vaadin.ui.Button.ClickEvent;
import com.vaadin.ui.Grid;
import com.vaadin.ui.Grid.Column;
import com.vaadin.ui.Label;
import com.vaadin.ui.Panel;
import com.vaadin.ui.UI;
import com.vaadin.ui.VerticalLayout;

import crypto.poloniex.wamp.swing.PoloWAMPController;
import crypto.poloniex.wamp.swing.PoloWAMPController.ConnectionStateObservable;
import crypto.poloniex.wamp.swing.PoloWAMPController.TableCell;
import crypto.poloniex.wamp.swing.PoloWAMPController.TableCellObservable;
import crypto.poloniex.wamp.swing.PoloWAMPController.TableRow;
import crypto.poloniex.wamp.swing.PoloWAMPController.TableRowObservable;

@SuppressWarnings("serial")
@Theme("polowampvaadin")
@Push
public class PolowampvaadinUI extends UI {

	public Button connectButton;
	public Button disconnectButton;
	public Label connectionStatus;
	public Grid exchangeGrid;
	public PoloWAMPController controller;
	public Observer connectionStateObserver;
	public Observer tableCellObserver;
	public IndexedContainer exchangeContainer;
	private String connectionState;
	private boolean push;
	
	public static final String DISCONNECTED = "Disconnected";
	public static final String CONNECTING = "Connecting...";
	public static final String CONNECTED = "Connected";
	@WebServlet(value = "/*", asyncSupported = true)
	@VaadinServletConfiguration(productionMode = false, ui = PolowampvaadinUI.class, widgetset = "com.example.polowampvaadin.widgetset.PolowampvaadinWidgetset")
	public static class Servlet extends VaadinServlet {
	}

	private class Pusher implements Runnable {
		public void run() {
			while(true) {
				if(getPush()) {
					setPush(false);
					push();
				}
				try {
				Thread.sleep(200);
				}
				catch (Exception e) {
					System.err.println("Pusher thread was interrupted!");
				}
			}
		}
	}
	public PolowampvaadinUI() {
		super();
		setPush(false);
		this.controller = new PoloWAMPController();
		connectionStateObserver = new Observer(){
			public void update(Observable obs, Object obj) {
				String newState = ((ConnectionStateObservable)obs).getState(); 
				if(!newState.equals(connectionState)) {
					connectionState = newState;
					connectionStatus.setValue(newState);
					setPush(true);
				}
			}
		};
		tableCellObserver = new Observer(){
			public void update(Observable obs, Object obj) {
				TableCellObservable tcobs = (TableCellObservable) obs;
				TableCell tc = tcobs.getTableCell();
				Item item = exchangeContainer.getItem(tc.rowId);
				if(item == null) {
					item = exchangeContainer.addItem(tc.rowId);
					Property codeP = item.getItemProperty("CODE");
					codeP.setValue(tc.rowId);
				}
				Property p = item.getItemProperty(tc.colId); 
				if(p == null) {
					p = new ObjectProperty<Double>(tc.value);
					item.addItemProperty(tc.colId, p);
				}
				else {
					p.setValue(tc.value);
				}
				setPush(true);
			}
		};
		exchangeContainer = new IndexedContainer();
		exchangeContainer.addContainerProperty("CODE", String.class, null);
		for(String colName : controller.getColNames()) {
			exchangeContainer.addContainerProperty(colName, Double.class, null);
		}
		
//		for(String code : PoloWAMPController.MAKER_CURRENCIES) {
//			exchangeContainer.addContainerProperty(code, Double.class, 0);
//		}
//		for(String code : PoloWAMPController.MAKER_CURRENCIES) {
//			for(String innerCode : PoloWAMPController.MAKER_CURRENCIES) {
//				if(!innerCode.equals(code)) {
//					exchangeContainer.addContainerProperty(code + "+" + innerCode, Double.class, 0);
//				}
//			}
//		}
		
		
		controller.addConnectionStateobserver(connectionStateObserver);
		controller.addTableCellObserver(tableCellObserver);
		connectionState = DISCONNECTED;
		new Thread(new Pusher()).start();
		VaadinSession.getCurrent().setConverterFactory(new MyConverterFactory());
	}
	
	@Override
	protected void init(VaadinRequest request) {
		final VerticalLayout layout = new VerticalLayout();
		layout.setMargin(true);
		setContent(layout);
		
		connectButton = new Button("Connect");
		connectButton.addClickListener(new Button.ClickListener() {
			public void buttonClick(ClickEvent event) {
				controller.startPriceStream();
			}
		});

		disconnectButton = new Button("Disconnect");
		disconnectButton.addClickListener(new Button.ClickListener() {
			public void buttonClick(ClickEvent event) {
				controller.stopPriceStream();
			}
		});
		
		connectionStatus = new Label(connectionState);
		connectionStatus.setImmediate(true);
		Panel buttonPanel = new Panel();
		HorizontalLayout controlLayout = new HorizontalLayout();
		
		controlLayout.addComponent(connectButton);
		controlLayout.addComponent(disconnectButton);
		controlLayout.addComponent(connectionStatus);
		buttonPanel.setContent(controlLayout);
		layout.addComponent(buttonPanel);
		exchangeGrid = new Grid("X RATES");
		exchangeGrid.setContainerDataSource(exchangeContainer);
		exchangeGrid.setWidth(1600,Unit.PIXELS);
		exchangeGrid.setHeight(800, Unit.PIXELS);
		for(Column col : exchangeGrid.getColumns()) {
			col.setWidth(150);
		}
		layout.addComponent(exchangeGrid);
	}
	
	private synchronized void setPush(boolean push) {
		this.push = push;
	}
	
	private synchronized boolean getPush() {
		return push;
	}
	
	public class MyStringToDoubleConverter extends StringToDoubleConverter {

	    @Override
	    protected NumberFormat getFormat(Locale locale) {
	        NumberFormat format = super.getFormat(locale);
	        format.setGroupingUsed(false);
	        format.setMaximumFractionDigits(10);
	        format.setMinimumFractionDigits(10);
	        return format;
	    }
	}
	
	public class MyConverterFactory extends DefaultConverterFactory {
	    @Override
	    protected <PRESENTATION, MODEL> Converter<PRESENTATION, MODEL> findConverter(
	            Class<PRESENTATION> presentationType, Class<MODEL> modelType) {
	        // Handle String <-> Double
	        if (presentationType == String.class && modelType == Double.class) {
	            return (Converter<PRESENTATION, MODEL>) new MyStringToDoubleConverter();
	        }
	        // Let default factory handle the rest
	        return super.findConverter(presentationType, modelType);
	    }
	}
}