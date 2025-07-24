import React, { useState, useEffect, useCallback, useRef } from 'react';

const SurvivalRacingGame = () => {
  // Game state
  const [playerPos, setPlayerPos] = useState({ x: 50, y: 400 });
  const [calories, setCalories] = useState(2000);
  const [inventory, setInventory] = useState([]);
  const [distanceTraveled, setDistanceTraveled] = useState(0);
  const [gameTime, setGameTime] = useState(0);
  const [movementMode, setMovementMode] = useState('walk');
  const [keysPressed, setKeysPressed] = useState(new Set());
  const [isResting, setIsResting] = useState(false);
  const [restTime, setRestTime] = useState(0);
  const [gameOver, setGameOver] = useState(false);
  const [won, setWon] = useState(false);
  const [floatingText, setFloatingText] = useState([]);
  const [collectedFoodAnimation, setCollectedFoodAnimation] = useState(null);

  // Refs for smooth updates
  const lastUpdateTime = useRef(Date.now());

  // Game constants
  const FINISH_DISTANCE = 2000;
  const BASE_CALORIE_BURN = 1.2; // per second when idle
  const REST_BURN_REDUCTION = 0.4; // 60% reduction when resting

  // Movement modes with strategic tradeoffs
  const MOVEMENT_MODES = {
    walk: { speed: 1.8, calorieMultiplier: 2.5, label: 'Walk', color: 'green' },
    jog: { speed: 3.2, calorieMultiplier: 5, label: 'Jog', color: 'yellow' },
    sprint: { speed: 5, calorieMultiplier: 9, label: 'Sprint', color: 'red' }
  };

  const TERRAIN_MULTIPLIERS = {
    grass: 1.0,
    mud: 1.4,
    water: 1.6,
    rock: 1.2
  };

  // Enhanced food placement for better strategic routing
  const [foodItems] = useState([
    // Early game - essential pickups on safe terrain
    { id: 1, x: 120, y: 380, type: 'berries', calories: 250, collected: false },
    { id: 2, x: 200, y: 420, type: 'roots', calories: 400, collected: false },
    
    // Mid-route choices - valuable food in water zones
    { id: 3, x: 620, y: 460, type: 'fish', calories: 1200, collected: false }, // in water
    { id: 4, x: 650, y: 450, type: 'fish', calories: 1200, collected: false }, // in water
    { id: 5, x: 320, y: 460, type: 'fish', calories: 1000, collected: false }, // in mud
    { id: 6, x: 350, y: 470, type: 'roots', calories: 600, collected: false }, // in mud
    
    // Safe path alternatives (lower value)
    { id: 7, x: 450, y: 380, type: 'berries', calories: 300, collected: false },
    { id: 8, x: 520, y: 390, type: 'mushrooms', calories: 350, collected: false },
    
    // Late game - high value in dangerous zones
    { id: 9, x: 1200, y: 460, type: 'fish', calories: 1200, collected: false }, // in water
    { id: 10, x: 1250, y: 470, type: 'fish', calories: 1200, collected: false }, // in water
    { id: 11, x: 1000, y: 460, type: 'roots', calories: 600, collected: false }, // in mud
    { id: 12, x: 1050, y: 470, type: 'mushrooms', calories: 500, collected: false }, // in mud
    
    // Final stretch - decision points
    { id: 13, x: 1500, y: 400, type: 'berries', calories: 300, collected: false },
    { id: 14, x: 1700, y: 390, type: 'roots', calories: 500, collected: false },
    { id: 15, x: 1850, y: 380, type: 'mushrooms', calories: 400, collected: false },
  ]);

  // Terrain zones with strategic placement
  const getTerrainType = (x, y) => {
    // Mud zones - multiple patches across the course
    if ((x > 250 && x < 450 && y > 440) || 
        (x > 950 && x < 1150 && y > 440) ||
        (x > 1400 && x < 1600 && y > 440)) return 'mud';
    // Water crossings - high value food zones
    if ((x > 580 && x < 720 && y > 430) ||
        (x > 1180 && x < 1320 && y > 430)) return 'water';
    // Rocky shortcut areas
    if ((x > 650 && x < 800 && y < 340) ||
        (x > 1250 && x < 1400 && y < 340)) return 'rock';
    return 'grass';
  };

  // Calculate metabolic effects from hunger
  const getHungerEffects = () => {
    const deficit = Math.max(0, 2000 - calories);
    return {
      vignette: Math.min(0.8, deficit / 2500),
      speedReduction: deficit > 800 ? Math.min(0.6, deficit / 3000) : 0,
      isDangerous: deficit > 1200,
      isCritical: deficit > 1600
    };
  };

  // Add floating text for feedback
  const addFloatingText = (text, color = 'white', x = playerPos.x, y = playerPos.y) => {
    const id = Date.now() + Math.random();
    setFloatingText(prev => [...prev, { id, text, color, x, y, opacity: 1 }]);
    
    setTimeout(() => {
      setFloatingText(prev => prev.filter(t => t.id !== id));
    }, 2000);
  };

  // Enhanced game loop with better timing
  useEffect(() => {
    let animationFrame;
    
    const gameLoop = () => {
      if (gameOver) return;

      const now = Date.now();
      const deltaTime = (now - lastUpdateTime.current) / 1000; // seconds
      lastUpdateTime.current = now;

      setGameTime(prev => prev + deltaTime);
      
      // Enhanced calorie burn calculation
      const terrain = getTerrainType(playerPos.x, playerPos.y);
      const terrainMultiplier = TERRAIN_MULTIPLIERS[terrain];
      
      // Check if player is actively moving based on keys pressed
      const isCurrentlyMoving = keysPressed.size > 0 && !isResting;
      
      let burnRate;
      
      if (isResting) {
        burnRate = BASE_CALORIE_BURN * REST_BURN_REDUCTION;
        setRestTime(prev => prev + deltaTime);
      } else if (isCurrentlyMoving) {
        const mode = MOVEMENT_MODES[movementMode];
        burnRate = BASE_CALORIE_BURN * mode.calorieMultiplier * terrainMultiplier;
        setRestTime(0);
        
        // Debug logging
        if (movementMode === 'sprint') {
          console.log(`SPRINTING: ${BASE_CALORIE_BURN} * ${mode.calorieMultiplier} * ${terrainMultiplier} = ${burnRate} cal/sec`);
        }
      } else {
        // Standing still but not resting
        burnRate = BASE_CALORIE_BURN;
      }
      
      setCalories(prev => {
        const newCalories = prev - burnRate * deltaTime;
        
        // Add warning text at critical thresholds
        if (prev > 500 && newCalories <= 500) {
          addFloatingText('CRITICALLY LOW ENERGY!', 'red');
        } else if (prev > 1000 && newCalories <= 1000) {
          addFloatingText('Low Energy Warning', 'orange');
        }
        
        if (newCalories <= 0) {
          setGameOver(true);
          addFloatingText('STARVED!', 'red');
        }
        return Math.max(0, newCalories);
      });

      // Check for food collection with enhanced feedback
      foodItems.forEach(food => {
        if (!food.collected && 
            Math.abs(playerPos.x - food.x) < 25 && 
            Math.abs(playerPos.y - food.y) < 25) {
          food.collected = true;
          setInventory(prev => [...prev, food]);
          setCollectedFoodAnimation({ x: food.x, y: food.y, type: food.type });
          addFloatingText('Found Food!', 'lightgreen', food.x, food.y);
          
          setTimeout(() => setCollectedFoodAnimation(null), 1000);
        }
      });

      // Check win condition
      if (distanceTraveled >= FINISH_DISTANCE) {
        setWon(true);
        setGameOver(true);
        addFloatingText('VICTORY!', 'gold');
      }

      // Update floating text
      setFloatingText(prev => prev.map(text => ({
        ...text,
        y: text.y - 20 * deltaTime,
        opacity: text.opacity - deltaTime * 0.5
      })).filter(text => text.opacity > 0));

      animationFrame = requestAnimationFrame(gameLoop);
    };

    animationFrame = requestAnimationFrame(gameLoop);
    return () => cancelAnimationFrame(animationFrame);
  }, [playerPos, keysPressed, isResting, movementMode, gameOver, distanceTraveled, foodItems]);

  // Enhanced movement with different speeds
  const movePlayer = useCallback((direction) => {
    if (gameOver) return;
    
    const hungerEffects = getHungerEffects();
    const mode = MOVEMENT_MODES[movementMode];
    const baseSpeed = mode.speed * (1 - hungerEffects.speedReduction);
    
    setPlayerPos(prev => {
      let newPos = { ...prev };
      const moveDistance = baseSpeed;
      
      switch(direction) {
        case 'left':
          newPos.x = Math.max(10, prev.x - moveDistance);
          break;
        case 'right':
          newPos.x = Math.min(1990, prev.x + moveDistance);
          setDistanceTraveled(prevDist => prevDist + moveDistance);
          break;
        case 'up':
          newPos.y = Math.max(300, prev.y - moveDistance);
          break;
        case 'down':
          newPos.y = Math.min(500, prev.y + moveDistance);
          break;
      }
      return newPos;
    });
  }, [gameOver, movementMode]);

  // Rest mechanic
  const startResting = () => {
    if (keysPressed.size === 0 && !isResting) {
      setIsResting(true);
      addFloatingText('Resting...', 'lightblue');
    }
  };

  const stopResting = () => {
    if (isResting) {
      setIsResting(false);
      addFloatingText(`Rested for ${restTime.toFixed(1)}s`, 'lightblue');
    }
  };

  // Eat food with enhanced feedback
  const eatFood = (foodId) => {
    const food = inventory.find(f => f.id === foodId);
    if (food) {
      const caloriesGained = food.calories;
      const newCalories = Math.min(2500, calories + caloriesGained);
      const actualGain = newCalories - calories;
      
      setCalories(newCalories);
      setInventory(prev => prev.filter(f => f.id !== foodId));
      addFloatingText(`+${actualGain} calories`, 'lightgreen');
      
      if (actualGain < caloriesGained) {
        addFloatingText(`Wasted ${caloriesGained - actualGain}`, 'orange');
      }
    }
  };

  // Enhanced keyboard controls with proper key tracking
  useEffect(() => {
    const handleKeyDown = (e) => {
      const key = e.key.toLowerCase();
      
      // Movement keys
      if (['a', 'arrowleft', 'd', 'arrowright', 'w', 'arrowup', 's', 'arrowdown'].includes(key)) {
        setKeysPressed(prev => new Set([...prev, key]));
        setIsResting(false);
        
        switch(key) {
          case 'a':
          case 'arrowleft':
            movePlayer('left');
            break;
          case 'd':
          case 'arrowright':
            movePlayer('right');
            break;
          case 'w':
          case 'arrowup':
            movePlayer('up');
            break;
          case 's':
          case 'arrowdown':
            movePlayer('down');
            break;
        }
      }
      
      // Mode switching
      if (key === '1') {
        setMovementMode('walk');
        addFloatingText('Walking', 'lightblue');
      } else if (key === '2') {
        setMovementMode('jog');
        addFloatingText('Jogging', 'yellow');
      } else if (key === '3') {
        setMovementMode('sprint');
        addFloatingText('Sprinting', 'red');
      }
      
      // Rest
      if (key === ' ') {
        e.preventDefault();
        if (isResting) {
          stopResting();
        } else {
          startResting();
        }
      }
    };

    const handleKeyUp = (e) => {
      const key = e.key.toLowerCase();
      
      if (['a', 'arrowleft', 'd', 'arrowright', 'w', 'arrowup', 's', 'arrowdown'].includes(key)) {
        setKeysPressed(prev => {
          const newSet = new Set(prev);
          newSet.delete(key);
          return newSet;
        });
      }
      
      if (e.key === ' ') {
        e.preventDefault();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    window.addEventListener('keyup', handleKeyUp);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
      window.removeEventListener('keyup', handleKeyUp);
    };
  }, [movePlayer, isResting, restTime]);

  const hungerEffects = getHungerEffects();
  const currentTerrain = getTerrainType(playerPos.x, playerPos.y);
  const progressPercent = (distanceTraveled / FINISH_DISTANCE) * 100;
  const currentMode = MOVEMENT_MODES[movementMode];

  // Calculate current burn rate for display
  const getCurrentBurnRate = () => {
    const terrainMultiplier = TERRAIN_MULTIPLIERS[currentTerrain];
    const isCurrentlyMoving = keysPressed.size > 0 && !isResting;
    
    if (isResting) return BASE_CALORIE_BURN * REST_BURN_REDUCTION;
    if (isCurrentlyMoving) return BASE_CALORIE_BURN * currentMode.calorieMultiplier * terrainMultiplier;
    return BASE_CALORIE_BURN;
  };

  return (
    <div className="w-full h-screen bg-gradient-to-b from-blue-200 to-green-200 overflow-hidden relative">
      {/* Enhanced hunger vignette effect */}
      {hungerEffects.vignette > 0 && (
        <div 
          className="absolute inset-0 pointer-events-none z-10"
          style={{
            background: `radial-gradient(circle, transparent 30%, rgba(0,0,0,${hungerEffects.vignette}) 80%)`,
          }}
        />
      )}

      {/* Enhanced HUD */}
      <div className="absolute top-4 left-4 bg-black bg-opacity-80 text-white p-4 rounded-lg z-20 min-w-72">
        <div className="space-y-2">
          <div className={`text-xl font-bold ${calories < 400 ? 'text-red-400 animate-pulse' : calories < 800 ? 'text-yellow-400' : 'text-green-400'}`}>
            Calories: {Math.round(calories)} / 2000
          </div>
          <div className="text-sm">Burn Rate: {getCurrentBurnRate().toFixed(1)} cal/sec</div>
          <div className="text-sm">Distance: {Math.round(distanceTraveled)}m / {FINISH_DISTANCE}m</div>
          <div className="text-sm">Terrain: {currentTerrain} ({TERRAIN_MULTIPLIERS[currentTerrain]}x)</div>
          <div className="text-sm">Time: {Math.round(gameTime)}s</div>
          
          {/* Movement mode indicator */}
          <div className="flex items-center space-x-2">
            <span className="text-sm">Mode:</span>
            <span className={`text-sm font-bold text-${currentMode.color}-400`}>
              {currentMode.label} ({currentMode.speed.toFixed(1)} speed, {currentMode.calorieMultiplier}x burn)
            </span>
          </div>
          
          {/* Status indicators */}
          {isResting && (
            <div className="text-blue-400 text-sm font-bold animate-pulse">
              😴 RESTING ({restTime.toFixed(1)}s)
            </div>
          )}
          {hungerEffects.isCritical && (
            <div className="text-red-400 text-sm font-bold animate-pulse">⚠ CRITICAL STARVATION</div>
          )}
          {hungerEffects.isDangerous && !hungerEffects.isCritical && (
            <div className="text-yellow-400 text-sm font-bold animate-pulse">⚠ VERY HUNGRY</div>
          )}
        </div>
      </div>

      {/* Progress bar */}
      <div className="absolute top-4 right-4 w-48 bg-gray-800 rounded-full h-6 z-20">
        <div 
          className="bg-blue-500 h-6 rounded-full transition-all duration-300"
          style={{ width: `${progressPercent}%` }}
        />
        <div className="text-white text-xs text-center mt-1">Progress: {Math.round(progressPercent)}%</div>
      </div>

      {/* Enhanced inventory */}
      <div className="absolute top-4 left-1/2 transform -translate-x-1/2 bg-black bg-opacity-80 text-white p-3 rounded-lg z-20 min-w-64">
        <div className="text-sm font-bold mb-2">Inventory ({inventory.length}/6)</div>
        <div className="flex flex-wrap gap-2">
          {inventory.map(food => (
            <button
              key={food.id}
              onClick={() => eatFood(food.id)}
              className="bg-green-600 hover:bg-green-500 px-2 py-1 rounded text-xs transition-colors"
              title={`${food.type} - ${food.calories} calories`}
            >
              {food.type} ({food.calories})
            </button>
          ))}
          {inventory.length === 0 && <span className="text-gray-400 text-xs">Empty - Find red food dots!</span>}
        </div>
      </div>

      {/* Game world */}
      <div className="relative w-full h-full">
        {/* Enhanced terrain visualization */}
        <div className="absolute inset-0">
          {/* Mud zones */}
          <div className="absolute bg-yellow-600 bg-opacity-40 border-2 border-yellow-700" style={{
            left: '250px', top: '440px', width: '200px', height: '60px'
          }}>
            <div className="text-yellow-800 text-xs font-bold p-1">MUD 1.4x</div>
          </div>
          <div className="absolute bg-yellow-600 bg-opacity-40 border-2 border-yellow-700" style={{
            left: '950px', top: '440px', width: '200px', height: '60px'
          }}>
            <div className="text-yellow-800 text-xs font-bold p-1">MUD 1.4x</div>
          </div>
          <div className="absolute bg-yellow-600 bg-opacity-40 border-2 border-yellow-700" style={{
            left: '1400px', top: '440px', width: '200px', height: '60px'
          }}>
            <div className="text-yellow-800 text-xs font-bold p-1">MUD 1.4x</div>
          </div>
          
          {/* Water zones */}
          <div className="absolute bg-blue-600 bg-opacity-40 border-2 border-blue-700" style={{
            left: '580px', top: '430px', width: '140px', height: '70px'
          }}>
            <div className="text-blue-800 text-xs font-bold p-1">WATER 1.6x</div>
          </div>
          <div className="absolute bg-blue-600 bg-opacity-40 border-2 border-blue-700" style={{
            left: '1180px', top: '430px', width: '140px', height: '70px'
          }}>
            <div className="text-blue-800 text-xs font-bold p-1">WATER 1.6x</div>
          </div>
          
          {/* Rock zones */}
          <div className="absolute bg-gray-600 bg-opacity-40 border-2 border-gray-700" style={{
            left: '650px', top: '300px', width: '150px', height: '40px'
          }}>
            <div className="text-gray-800 text-xs font-bold p-1">ROCK 1.2x</div>
          </div>
          <div className="absolute bg-gray-600 bg-opacity-40 border-2 border-gray-700" style={{
            left: '1250px', top: '300px', width: '150px', height: '40px'
          }}>
            <div className="text-gray-800 text-xs font-bold p-1">ROCK 1.2x</div>
          </div>
        </div>

        {/* Enhanced food items */}
        {foodItems.map(food => !food.collected && (
          <div
            key={food.id}
            className="absolute w-5 h-5 bg-red-500 rounded-full animate-pulse border-2 border-red-700 flex items-center justify-center"
            style={{ left: `${food.x}px`, top: `${food.y}px` }}
            title={`${food.type} - ${food.calories} calories`}
          >
            <span className="text-white text-xs font-bold">{food.calories}</span>
          </div>
        ))}

        {/* Food collection animation */}
        {collectedFoodAnimation && (
          <div
            className="absolute w-8 h-8 bg-green-400 rounded-full animate-ping"
            style={{ 
              left: `${collectedFoodAnimation.x - 6}px`, 
              top: `${collectedFoodAnimation.y - 6}px` 
            }}
          />
        )}

        {/* Finish line */}
        <div 
          className="absolute w-3 h-full bg-yellow-400 border-2 border-yellow-600"
          style={{ left: `${50 + (FINISH_DISTANCE * 0.9)}px` }}
        />
        <div 
          className="absolute text-yellow-700 font-bold transform -rotate-90 text-lg"
          style={{ left: `${50 + (FINISH_DISTANCE * 0.9) + 15}px`, top: '350px' }}
        >
          FINISH LINE
        </div>

        {/* Enhanced player with state visualization */}
        <div
          className={`absolute w-8 h-8 rounded-full transition-all duration-150 border-2 ${
            isResting ? 'bg-blue-400 border-blue-600 animate-pulse' :
            movementMode === 'sprint' ? 'bg-red-500 border-red-700' :
            movementMode === 'jog' ? 'bg-yellow-500 border-yellow-700' :
            'bg-green-500 border-green-700'
          } ${keysPressed.size > 0 ? 'animate-bounce' : ''}`}
          style={{ 
            left: `${playerPos.x - 4}px`, 
            top: `${playerPos.y - 4}px`,
            opacity: hungerEffects.vignette > 0.6 ? 0.6 : 1,
            transform: `scale(${hungerEffects.speedReduction > 0.3 ? 0.8 : 1})`
          }}
        />

        {/* Floating text */}
        {floatingText.map(text => (
          <div
            key={text.id}
            className="absolute text-sm font-bold pointer-events-none z-30"
            style={{
              left: `${text.x}px`,
              top: `${text.y}px`,
              color: text.color,
              opacity: text.opacity,
              textShadow: '1px 1px 2px rgba(0,0,0,0.8)'
            }}
          >
            {text.text}
          </div>
        ))}
      </div>

      {/* Enhanced controls */}
      <div className="absolute bottom-4 left-4 bg-black bg-opacity-80 text-white p-4 rounded-lg z-20 max-w-md">
        <div className="text-sm space-y-1">
          <div className="font-bold text-green-400">MOVEMENT:</div>
          <div>WASD/Arrows: Move • 1/2/3: Walk/Jog/Sprint</div>
          <div>SPACE: Hold to Rest (60% less burn)</div>
          <div className="font-bold text-red-400 mt-2">STRATEGY:</div>
          <div>• Red dots = Food (walk near to collect)</div>
          <div>• Click inventory items to eat</div>
          <div>• Different terrains cost more energy</div>
          <div>• Sprint for speed, walk to conserve energy</div>
          <div>• Rest when safe to recover efficiency</div>
        </div>
      </div>

      {/* Enhanced Game Over Screen */}
      {gameOver && (
        <div className="absolute inset-0 bg-black bg-opacity-90 flex items-center justify-center z-30">
          <div className="bg-white p-8 rounded-lg text-center max-w-md">
            <h2 className="text-4xl font-bold mb-4">
              {won ? '🏆 SURVIVAL VICTORY!' : '💀 STARVED TO DEATH'}
            </h2>
            <div className="space-y-3 text-lg">
              <div className="grid grid-cols-2 gap-4 text-left">
                <div>Distance:</div><div className="font-bold">{Math.round(distanceTraveled)}m / {FINISH_DISTANCE}m</div>
                <div>Final Calories:</div><div className="font-bold">{Math.round(calories)}</div>
                <div>Survival Time:</div><div className="font-bold">{Math.round(gameTime)}s</div>
                <div>Food Collected:</div><div className="font-bold">{foodItems.filter(f => f.collected).length}/{foodItems.length}</div>
              </div>
              
              {won && (
                <div className="text-green-600 font-bold mt-4">
                  You successfully balanced speed and energy management!
                </div>
              )}
              {!won && (
                <div className="text-red-600 font-bold mt-4">
                  {distanceTraveled < 200 ? "Burned energy too fast early on!" :
                   distanceTraveled < 500 ? "Should have collected more food mid-route!" :
                   "So close! Better energy management in the final stretch!"}
                </div>
              )}
            </div>
            <button 
              onClick={() => window.location.reload()}
              className="mt-6 bg-blue-600 text-white px-8 py-3 rounded-lg hover:bg-blue-500 text-lg font-bold"
            >
              Race Again
            </button>
          </div>
        </div>
      )}
    </div>
  );
};

export default SurvivalRacingGame;