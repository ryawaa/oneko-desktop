import Cocoa

let spriteSets: [String: [(Int, Int)]] = [
  "idle": [(-3, -3)],
  "alert": [(-7, -3)],
  "scratchSelf": [(-5, 0), (-6, 0), (-7, 0)],
  "scratchWallS": [(0, 0), (0, -1)],
  "scratchWallN": [(-7, -1), (-6, -2)],
  "scratchWallE": [(-2, -2), (-2, -3)],
  "scratchWallW": [(-4, 0), (-4, -1)],
  "tired": [(-3, -2)],
  "sleeping": [(-2, 0), (-2, -1)],
  "S": [(-1, -2), (-1, -3)],
  "SE": [(0, -2), (0, -3)],
  "E": [(-3, 0), (-3, -1)],
  "NE": [(-5, -1), (-5, -2)],
  "N": [(-6, -3), (-7, -2)],
  "NW": [(-5, -3), (-6, -1)],
  "W": [(-4, -2), (-4, -3)],
  "SW": [(-1, 0), (-1, -1)],
]

class OverlayWindow: NSWindow {
  override var canBecomeKey: Bool { return true }
  override var canBecomeMain: Bool { return true }

  init() {
    super.init(
      contentRect: NSRect(x: 100, y: 100, width: 32, height: 32),
      styleMask: .borderless,
      backing: .buffered,
      defer: false
    )
    self.isOpaque = false
    self.backgroundColor = .clear
    self.ignoresMouseEvents = false  
    self.level = .mainMenu + 1
    self.collectionBehavior = [.canJoinAllSpaces, .stationary]
  }
}


class CatView: NSImageView {
  var parentApp: AppDelegate?

  override func rightMouseDown(with event: NSEvent) {
    
    let menu = NSMenu()
    menu.addItem(NSMenuItem(title: "Close oneko", action: #selector(closeOneko), keyEquivalent: ""))
    NSMenu.popUpContextMenu(menu, with: event, for: self)
  }

  @objc func closeOneko() {
    NSApp.terminate(nil)
  }
}

class AppDelegate: NSObject, NSApplicationDelegate {
  var window: OverlayWindow?
  var imageView: CatView!
  var mouseMonitor: Any?

  var nekoPosX: CGFloat = 100
  var nekoPosY: CGFloat = 100
  var mousePosX: CGFloat = 0
  var mousePosY: CGFloat = 0

  let nekoSpeed: CGFloat = 10
  var frameCount: Int = 0
  var idleTime: Int = 0
  var idleAnimation: String? = nil
  var idleAnimationFrame: Int = 0

  var spriteSheet: NSImage?
  var timer: Timer?

  func applicationDidFinishLaunching(_ notification: Notification) {
    
    window = OverlayWindow()

    let spritePath = FileManager.default.currentDirectoryPath + "/oneko.gif"
    guard let sheet = NSImage(contentsOfFile: spritePath) else {
      print("Could not load oneko.gif from current directory.")
      NSApp.terminate(nil)
      return
    }
    spriteSheet = sheet

    imageView = CatView(frame: NSRect(x: 0, y: 0, width: 32, height: 32))
    imageView.parentApp = self
    window?.contentView?.addSubview(imageView)
    window?.setContentSize(NSSize(width: 32, height: 32))
    window?.makeKeyAndOrderFront(nil)

    
    mouseMonitor = NSEvent.addGlobalMonitorForEvents(matching: .mouseMoved) { [weak self] event in
      guard let self = self else { return }
      let mouseLoc = NSEvent.mouseLocation
      self.mousePosX = mouseLoc.x
      self.mousePosY = mouseLoc.y
    }

    NSEvent.addLocalMonitorForEvents(matching: .mouseMoved) { event in
      let mouseLoc = NSEvent.mouseLocation
      self.mousePosX = mouseLoc.x
      self.mousePosY = mouseLoc.y
      return event
    }

    timer = Timer.scheduledTimer(
      timeInterval: 0.1, target: self, selector: #selector(updateFrame), userInfo: nil,
      repeats: true)
  }

  @objc func updateFrame() {
    frame()
  }

  func frame() {
    frameCount += 1
    let diffX = nekoPosX - mousePosX
    let diffY = nekoPosY - mousePosY
    let distance = sqrt(diffX * diffX + diffY * diffY)

    if distance < nekoSpeed || distance < 48 {
      idle()
      return
    }

    idleAnimation = nil
    idleAnimationFrame = 0

    if idleTime > 1 {
      setSprite(name: "alert", frameIndex: 0)
      idleTime = min(idleTime, 7)
      idleTime -= 1
      return
    }

    let dx = diffX / distance
    let dy = diffY / distance

    let verticalDir = dy > 0.5 ? "N" : (dy < -0.5 ? "S" : "")
    let horizontalDir = dx > 0.5 ? "W" : (dx < -0.5 ? "E" : "")
    let direction = verticalDir + horizontalDir

    setSprite(name: direction.isEmpty ? "idle" : direction, frameIndex: frameCount)

    nekoPosX -= dx * nekoSpeed
    nekoPosY -= dy * nekoSpeed

    if let screen = NSScreen.main {
      nekoPosX = min(max(16, nekoPosX), screen.frame.maxX - 16)
      nekoPosY = min(max(16, nekoPosY), screen.frame.maxY - 16)
    }

    updateWindowPosition()
  }

  func idle() {
    idleTime += 1

    if idleTime > 10 && Int.random(in: 0..<200) == 0 && idleAnimation == nil {
      var available = ["sleeping", "scratchSelf"]
      if nekoPosX < 32 { available.append("scratchWallW") }
      if nekoPosY < 32 { available.append("scratchWallN") }
      if let screen = NSScreen.main {
        if nekoPosX > screen.frame.maxX - 32 { available.append("scratchWallE") }
        if nekoPosY > screen.frame.maxY - 32 { available.append("scratchWallS") }
      }
      idleAnimation = available.randomElement()
    }

    guard let idleAnim = idleAnimation else {
      setSprite(name: "idle", frameIndex: 0)
      return
    }

    switch idleAnim {
    case "sleeping":
      if idleAnimationFrame < 8 {
        setSprite(name: "tired", frameIndex: 0)
      } else {
        setSprite(name: "sleeping", frameIndex: idleAnimationFrame / 4)
      }
      if idleAnimationFrame > 192 {
        resetIdleAnimation()
      }
    case "scratchWallN", "scratchWallS", "scratchWallE", "scratchWallW", "scratchSelf":
      setSprite(name: idleAnim, frameIndex: idleAnimationFrame)
      if idleAnimationFrame > 9 {
        resetIdleAnimation()
      }
    default:
      setSprite(name: "idle", frameIndex: 0)
    }

    idleAnimationFrame += 1
  }

  func resetIdleAnimation() {
    idleAnimation = nil
    idleAnimationFrame = 0
  }

func setSprite(name: String, frameIndex: Int) {
    guard let frames = spriteSets[name], let sheet = spriteSheet else {
        setSprite(name: "idle", frameIndex: frameIndex)
        return
    }
    let frame = frames[frameIndex % frames.count]
    
    let offsetX = frame.0 * 32
    let offsetY = frame.1 * 32
 
    let cropX = -CGFloat(offsetX)
    let cropY = -CGFloat(offsetY)

    guard let cgImage = sheet.cgImage(forProposedRect: nil, context: nil, hints: nil) else { return }
    let sheetWidth = CGFloat(cgImage.width)
    let sheetHeight = CGFloat(cgImage.height)

    let srcRect = NSRect(x: cropX, y: cropY, width: 32, height: 32)

    if srcRect.minX < 0 || srcRect.minY < 0 || srcRect.maxX > sheetWidth || srcRect.maxY > sheetHeight {
        print("Warning: Attempting to crop outside the sprite sheet. Check the sprite offsets or sprite sheet size.")
        return
    }
    
    if let cropped = cgImage.cropping(to: srcRect) {
        let frameImage = NSImage(cgImage: cropped, size: NSSize(width: 32, height: 32))
        imageView.image = frameImage
    }
}

  func updateWindowPosition() {
    window?.setFrameOrigin(NSPoint(x: nekoPosX - 16, y: nekoPosY - 16))
  }

  func applicationWillTerminate(_ notification: Notification) {
    if let monitor = mouseMonitor {
      NSEvent.removeMonitor(monitor)
    }
    timer?.invalidate()
  }
}


let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate
app.run()
